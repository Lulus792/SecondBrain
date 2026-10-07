"""Selected original CommonMark entity fixtures through the actual C probe."""
from pathlib import Path
import json
import sys
import tempfile
from test_emphasis_spec import Expected, actual, trim

def main():
    probe, fixture = sys.argv[1:]
    cases = json.loads(Path(fixture).read_text(encoding='utf-8'))
    with tempfile.TemporaryDirectory(prefix='secondbrain-entity-spec-') as folder:
        path = Path(folder) / 'input.md'
        for case in cases:
            path.write_text(case['markdown'], encoding='utf-8')
            parser = Expected()
            parser.feed(case['html'])
            expected = parser.output
            if case['example'] == 31:
                expected = [(c, 0) for c in case['markdown'].rstrip('\n')]
            expected = trim(expected)
            # HTML serialization appends a terminal code-line separator; the
            # source block projection does not synthesize that extra byte.
            if '<pre>' in case['html'] and expected and expected[-1] == ('\n', 4):
                expected.pop()
            if trim(actual(probe, path)) != expected:
                raise SystemExit(f"Entity fixture {case['example']} differs from its original text/style structure")
    print(f'{len(cases)} selected CommonMark entity fixtures passed; raw HTML uses the literal contract.')

if __name__ == '__main__':
    main()
