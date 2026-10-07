"""Compare actual C projection against the pinned independent WHATWG map."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile

def main():
    probe, source = sys.argv[1:]
    entries = [(name, value['characters']) for name, value in json.loads(Path(source).read_text()).items() if name.endswith(';')]
    with tempfile.TemporaryDirectory(prefix='secondbrain-entities-') as folder:
        path = Path(folder) / 'input.md'
        path.write_text(' '.join(name for name, value in entries), encoding='utf-8')
        result = subprocess.check_output([probe, str(path)], text=True)
        blocks = [bytes.fromhex(line[2:]).decode('utf-8') for line in result.splitlines() if line.startswith('B ')]
        expected = ' '.join(value.replace('\r', ' ').replace('\n', ' ') for name, value in entries)
        if blocks != [expected]:
            raise SystemExit('C projection differs from the independent entity source')
        path.write_text(' '.join('`' + name + '`' for name, value in entries), encoding='utf-8')
        result = subprocess.check_output([probe, str(path)], text=True)
        blocks = [bytes.fromhex(line[2:]).decode('utf-8') for line in result.splitlines() if line.startswith('B ')]
        if blocks != [' '.join(name for name, value in entries)]:
            raise SystemExit('Code references were changed')
    print(f'{len(entries)} named entities match the source and remain literal in code.')

if __name__ == "__main__":
    main()
