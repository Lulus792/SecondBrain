#!/usr/bin/env python3
"""Publish bounded CI diagnostics without another shell/CMake path boundary."""
import argparse
from pathlib import Path
import re
import sys


def annotation(title, detail):
    escape = lambda value: value.replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
    sys.stdout.buffer.write(('::error title=' + escape(title) + '::' + escape(detail) + '\n').encode('utf-8'))
    sys.stdout.buffer.flush()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True)
    parser.add_argument('--title', default='Test entry')
    args = parser.parse_args()
    path = Path(args.log)
    if not path.is_file():
        annotation(args.title, 'No entry log exists: ' + str(path.resolve()))
        return
    detail = path.read_text(encoding='utf-8', errors='replace')
    fragments = []
    # Keep error context before later successful tests displace it from the tail.
    for match in re.finditer(r'(?m)^.*(?:FAIL:|ERROR:|AssertionError|ValueError|FileNotFoundError|UnicodeDecodeError|CMake Error).*$', detail):
        fragments.append(detail[match.start():match.start() + 700])
        if sum(map(len, fragments)) >= 3500:
            break
    annotation(args.title, '\n'.join(fragments)[:3500] + '\n' + detail[-3500:])


if __name__ == '__main__':
    main()
