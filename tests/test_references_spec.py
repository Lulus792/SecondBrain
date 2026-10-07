"""Compare original reference-link fixtures with actual C text/styles/targets."""
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote
import json
import subprocess
import sys
import tempfile
from test_emphasis_spec import Expected, actual, trim
class ReferenceText(Expected):
    def handle_starttag(self, tag, attrs):
        if tag not in ('p','h1','h2','h3','h4','h5','h6','li','pre','code','em','strong','a','img','blockquote','ul','ol'):
            self.handle_data(self.get_starttag_text())  # product's literal HTML contract
        else:
            super().handle_starttag(tag,attrs)
    def handle_endtag(self, tag):
        if tag == 'code' and self.pre and self.output and self.output[-1] == ('\n', 4):
            self.output.pop()  # terminal HTML code serializer newline
        super().handle_endtag(tag)
class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []
    def handle_starttag(self, tag, attrs):
        if tag == 'a': self.links.append(unquote(dict(attrs)['href']))
def main():
    probe, fixture = sys.argv[1:]
    cases = json.loads(Path(fixture).read_text(encoding='utf-8'))
    failures = []
    with tempfile.TemporaryDirectory(prefix='secondbrain-reference-spec-') as folder:
        path = Path(folder) / 'input.md'
        for case in cases:
            path.write_text(case['markdown'], encoding='utf-8')
            text, links = ReferenceText(), Links()
            text.feed(case['html']); links.feed(case['html'])
            # The product displays styled alternative text, rather than an HTML
            # image. These two originals have italic 'foo' in their alternatives.
            if case['example'] in (585,589):
                text.output=[(c,1 if i<3 else style) for i,(c,style) in enumerate(text.output)]
            got = trim(actual(probe, path))
            output = subprocess.check_output([probe, str(path), '--links'], text=True)
            destinations = [bytes.fromhex(line[2:]).decode('utf-8') for line in output.splitlines()]
            if got != trim(text.output) or destinations != links.links:
                failures.append(case['example'])
                print('REFERENCE FAIL', case['example'],repr(case['markdown']),got,trim(text.output),destinations,links.links)
    if failures: raise SystemExit(f'Reference fixtures failed: {failures}')
    print(f'{len(cases)} original reference fixtures passed for C text, style and targets.')
if __name__ == '__main__': main()
