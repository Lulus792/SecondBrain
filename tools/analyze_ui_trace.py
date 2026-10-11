#!/usr/bin/env python3
"""Distinguish prepared submissions from settled frames in the desktop trace."""
import argparse
import csv
import json
import math
from pathlib import Path


def analyze(path):
    with path.open(newline='', encoding='utf-8') as stream:
        rows = list(csv.DictReader(stream))
    if len(rows) != 272:
        raise ValueError('Expected four phases with 68 frames each')
    groups = {}
    for phase in ('idle', 'scroll', 'camera', 'switch'):
        selected = [row for row in rows if row['phase'] == phase]
        if [int(row['frame']) for row in selected] != list(range(68)):
            raise ValueError('Missing or unordered phase frames: ' + phase)
    for row in rows:
        serial = int(row['request_serial'])
        if row['present_ok'] != '1':
            raise ValueError('Failed renderer presentation')
        for name in ('reader_present', 'reader_pending', 'transitioning'):
            if row[name] not in ('0', '1'):
                raise ValueError('Invalid reader state: ' + name)
        if int(row['reader_text_commands']) < 0 or int(row['reader_image_commands']) < 0:
            raise ValueError('Invalid command count')
        if not 0 <= float(row['transition_progress']) <= 1:
            raise ValueError('Invalid transition progress')
        elapsed = float(row['request_elapsed_ms'])
        parts = [float(row[key + '_ms']) for key in ('input', 'layout', 'draw', 'present', 'apply', 'observe')]
        total = float(row['total_ms'])
        if not all(math.isfinite(value) for value in [elapsed, total, *parts]) or elapsed < 0 or any(value < 0 for value in parts) or abs(sum(parts) - total) > 0.5:
            raise ValueError('Invalid frame timing')
        group = groups.setdefault(serial, [])
        if group and (group[-1]['active_request'] != row['active_request']
                      or float(group[-1]['request_elapsed_ms']) > elapsed):
            raise ValueError('Request identity or monotonic clock changed')
        group.append(row)
    result = []
    for serial, group in groups.items():
        matching = [row for row in group if row['presented_note'] == row['active_request']
                    and row['reader_present'] == '1' and row['reader_pending'] == '0'
                    and int(row['reader_text_commands']) + int(row['reader_image_commands']) > 0]
        settled = [row for row in matching if row['transitioning'] == '0']
        result.append({'serial': serial, 'target': group[0]['active_request'],
                       'phase': group[0]['phase'], 'frame': int(group[0]['frame']),
                       'first_prepared_submission_ms': float(matching[0]['request_elapsed_ms']) if matching else None,
                       'first_settled_submission_ms': float(settled[0]['request_elapsed_ms']) if settled else None,
                       'observed_until_ms': float(group[-1]['request_elapsed_ms']),
                       'status': 'settled' if settled else 'prepared_not_settled' if matching else 'not_prepared_in_sample'})
    return {'frames': len(rows), 'requests': result,
            'model_path_changed_after_present': sum(row['presented_note'] != row['note_after'] for row in rows),
            'observer_max_ms': max(float(row['observe_ms']) for row in rows),
            'limitations': ['Prepared text/image commands and successful Present are internal renderer evidence',
                            'No compositor/display scanout timestamp or human readability measurement',
                            'Missing milestones remain null; rapid retargets or sample end may prevent settling']}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('trace', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = analyze(args.trace)
    payload = json.dumps(result, ensure_ascii=False, indent=2) + '\n'
    if args.output:
        with args.output.open('x', encoding='utf-8') as stream:
            stream.write(payload)
    else:
        print(payload, end='')


if __name__ == '__main__':
    main()
