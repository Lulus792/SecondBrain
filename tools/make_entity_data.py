#!/usr/bin/env python3
"""Generate own C lookup data from the pinned WHATWG entity map."""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'third_party/whatwg/2026-10-07/entities.json'
SHA256 = 'd741d877ac77c4194c4ad526b5b4a19aef8dfe411ab840a466891cdbb9f362e6'

def generate():
    data = SOURCE.read_bytes()
    if hashlib.sha256(data).hexdigest() != SHA256:
        raise ValueError('Entity source differs from its pinned original')
    entries = []
    for name, value in sorted(json.loads(data).items()):
        if not name.endswith(';'):
            continue
        points = value['codepoints']
        if not 1 <= len(points) <= 2 or ''.join(map(chr, points)) != value['characters']:
            raise ValueError('Invalid entity mapping')
        if len(name) > 33 or not name[1:-1].isascii() or not name[1:-1].isalnum():
            raise ValueError('Unsupported entity name')
        entries.append('    {"%s",0x%x,0x%x},' % (name, points[0], points[1] if len(points) == 2 else 0))
    if len(entries) != 2125:
        raise ValueError('Entity count differs from the reviewed source')
    return ('/* WHATWG entity data; BSD 3-Clause for code incorporation.\n'
            '   Copyright WHATWG (Apple, Google, Mozilla, Microsoft).\n'
            '   Source and licence: third_party/whatwg/README.md, third_party/licenses/WHATWG.txt. */\n'
            'static const EntityName entity_names[]={\n' + '\n'.join(entries) + '\n};\n')

if __name__ == '__main__':
    (ROOT / 'src/entity_names.inc').write_text(generate(), encoding='utf-8')
