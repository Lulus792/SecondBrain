"""Original CommonMark autolinks: compare text/styles and actual C destinations."""
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote
import json
import random
import re
import subprocess
import sys
import tempfile
from test_emphasis_spec import Expected, actual, trim

class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []
    def handle_starttag(self, tag, attrs):
        if tag == 'a':
            self.links.append(unquote(dict(attrs)['href']))

def main():
    probe, fixture = sys.argv[1:]
    cases = json.loads(Path(fixture).read_text(encoding='utf-8'))
    with tempfile.TemporaryDirectory(prefix='secondbrain-autolink-spec-') as folder:
        path = Path(folder) / 'input.md'
        for case in cases:
            path.write_text(case['markdown'], encoding='utf-8')
            text, links = Expected(), Links()
            text.feed(case['html'])
            links.feed(case['html'])
            got = trim(actual(probe, path))
            if got != trim(text.output):
                raise SystemExit(f"Autolink text/style fixture {case['example']}: {got!r} != {trim(text.output)!r}")
            output = subprocess.check_output([probe, str(path), '--links'], text=True)
            destinations = []
            for line in output.splitlines():
                if not line.startswith('L '):
                    raise ValueError('Unexpected link probe output: ' + line)
                destinations.append(bytes.fromhex(line[2:]).decode('utf-8'))
            if destinations != links.links:
                raise SystemExit(f"Autolink destination fixture {case['example']}: {destinations!r} != {links.links!r}")
        # Independent regex oracle for deterministic boundary/grammar inputs.
        local = r"[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+"
        domain = r"[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?"
        grammar = re.compile(local + '@' + domain + r'(?:\.' + domain + r')*')
        rng = random.Random(6068)
        addresses = []
        for _ in range(1000):
            left = ''.join(rng.choices("abcAZ09.!#$%&'*+/=?^_`{|}~-\\ ", k=rng.randrange(0,40)))
            right = '.'.join(''.join(rng.choices('azAZ09-_', k=rng.randrange(0,68))) for _ in range(rng.randrange(1,5)))
            addresses.append(left + '@' + right)
        path.write_text('\n\n'.join('<' + address + '>' for address in addresses), encoding='utf-8')
        output = subprocess.check_output([probe, str(path), '--links'], text=True)
        got = [bytes.fromhex(line[2:]).decode('utf-8') for line in output.splitlines()]
        expected = ['mailto:' + address for address in addresses if grammar.fullmatch(address)]
        if got != expected:
            raise SystemExit('Generated email destination grammar differs from the independent regex oracle')
    print(f'{len(cases)} original CommonMark autolink text/style/destination fixtures passed.')
    print('1000 generated email grammar inputs passed the independent regex oracle.')

if __name__ == '__main__':
    main()
