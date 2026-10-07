"""Compare actual C full/default folding against every pinned Unicode mapping."""
from pathlib import Path
import subprocess
import sys
import tempfile

def main():
    probe, source = sys.argv[1:]
    entries = []
    for line in Path(source).read_text(encoding='utf-8').splitlines():
        fields = [p.strip() for p in line.split('#', 1)[0].split(';')]
        if len(fields) >= 3 and fields[1] in ('C', 'F'):
            entries.append((chr(int(fields[0], 16)), ''.join(chr(int(cp, 16)) for cp in fields[2].split())))
    with tempfile.TemporaryDirectory(prefix='secondbrain-fold-data-') as folder:
        path = Path(folder) / 'fold.txt'
        path.write_text('\n'.join(cp for cp, _ in entries) + '\n', encoding='utf-8')
        output = subprocess.check_output([probe, str(path), '--fold'], text=True)
        got = []
        for line in output.splitlines():
            if not line.startswith('N '): raise ValueError('Unexpected folding probe output')
            got.append(bytes.fromhex(line[2:]).decode('utf-8'))
        expected = [mapping for _, mapping in entries]
        if got != expected: raise SystemExit('C full/default folding differs from original Unicode C/F mappings')
    print(f'{len(entries)} original Unicode 18.0.0 full/default fold mappings passed through actual C output.')

if __name__ == '__main__': main()
