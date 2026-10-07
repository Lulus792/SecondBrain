"""Compare actual C style spans with the pinned CommonMark emphasis examples."""
from html.parser import HTMLParser
from pathlib import Path
import json
import subprocess
import sys
import tempfile

class Expected(HTMLParser):
    def __init__(self):
        super().__init__()
        self.styles = []
        self.body = 0
        self.pre = False
        self.output = []

    def handle_starttag(self, tag, attrs):
        if tag in ('em', 'strong', 'code'):
            self.styles.append({'em': 1, 'strong': 2, 'code': 4}[tag])
        if tag in ('p', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'li'):
            self.body += 1
        if tag == 'pre':
            self.pre = True
        if tag == 'img':
            self.handle_data(dict(attrs).get('alt', ''))

    def handle_endtag(self, tag):
        if tag in ('em', 'strong', 'code'):
            self.styles.pop()
        if tag in ('p', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'li'):
            self.body -= 1
            self.output.append(('\n', 0))
        if tag == 'pre':
            self.pre = False
            self.output.append(('\n', 0))

    def handle_data(self, data):
        style = 0
        for value in self.styles:
            style |= value
        for char in data:
            if char == '\n' and not self.body and not self.pre:
                continue
            self.output.append((' ' if char == '\n' and not self.pre else char, style))

def actual(probe, path):
    output = subprocess.check_output([probe, str(path)], text=True)
    result, block, spans = [], b'', []
    def flush():
        for offset, length, style in spans:
            result.extend((c, style) for c in block[offset:offset + length].decode('utf-8'))
        if block:
            result.append(('\n', 0))
    for line in output.splitlines():
        if line.startswith('B '):
            flush()
            block, spans = bytes.fromhex(line[2:]), []
        elif line.startswith('S '):
            spans.append(tuple(map(int, line[2:].split())))
        else:
            raise ValueError('Unexpected probe output: ' + line)
    flush()
    return result

def trim(data):
    while data and data[-1] == ('\n', 0):
        data.pop()
    return data

def main():
    probe, fixture = sys.argv[1:]
    cases = json.loads(Path(fixture).read_text(encoding='utf-8'))
    failures = []
    with tempfile.TemporaryDirectory(prefix='secondbrain-emphasis-') as folder:
        path = Path(folder) / 'input.md'
        for case in cases:
            path.write_text(case['markdown'], encoding='utf-8')
            parser = Expected()
            parser.feed(case['html'])
            expected = parser.output
            # Product contract: raw HTML is displayed literally, never executed.
            if case['example'] in (475, 476, 477):
                expected = [(c, 0) for c in case['markdown'].rstrip('\n')]
            got = trim(actual(probe, path))
            if got != trim(expected):
                failures.append(case['example'])
                print('EMPHASIS FAIL', case['example'], repr(case['markdown']), got, expected)
    if failures:
        raise SystemExit(f'{len(failures)}/{len(cases)} emphasis fixtures failed: {failures}')
    print(f'{len(cases)} CommonMark emphasis fixtures passed; 3 raw-HTML cases use the documented literal display contract.')

if __name__ == '__main__':
    main()
