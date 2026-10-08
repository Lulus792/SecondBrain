#!/usr/bin/env python3
"""Reproduce the UI-only Unicode 18 UBA tables; never used by the app/build.

Uses the pinned upstream developer generator, with a checked 32-bit pairing
delta correction for new supplementary-plane mirror partners. No network I/O.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'third_party/ui/bidi18'
DATA = ROOT / 'third_party/unicode/18.0.0'
GENERATOR_SOURCES = {
    'PairingLookupGenerator.cpp': '1bdeff5f42f60085ba1998978f9f39a6710f3d31edcbde4df80a49f06184d34f',
    'PairingLookupGenerator.h': '5b7bf9ddb42ddef064caeaaf4448c0927490a30cdbe0bef6cec11f8ec7802f15',
}

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def reproduce(archive, cmake, check):
    manifest = json.loads((OUTPUT / 'manifest.json').read_text())
    if digest(archive) != manifest['archive_sha256']:
        raise ValueError('Not the pinned SheenBidi 3.0.0 archive')
    for name, expected in manifest['inputs'].items():
        if Path(name).name != name or digest(DATA / name) != expected:
            raise ValueError('Unreviewed Unicode source: ' + name)
    if set(manifest['outputs']) != {'BidiTypeLookup.c', 'PairingLookup.c'}:
        raise ValueError('Unexpected output set')
    with tempfile.TemporaryDirectory(prefix='secondbrain-bidi-') as directory:
        temporary = Path(directory)
        with tarfile.open(archive) as packed:
            packed.extractall(temporary, filter='data')
        source = temporary / 'SheenBidi-3.0.0'
        for name, expected in GENERATOR_SOURCES.items():
            path = source / 'Tools/Generator' / name
            if digest(path) != expected:
                raise ValueError('Unreviewed generator source: ' + name)
            text = path.read_text()
            text = text.replace('int16_t', 'int32_t').replace('SBInt16', 'SBInt32')
            text = text.replace('mirror - codePoint', '(int32_t)mirror - (int32_t)codePoint')
            text = text.replace('m_differencesSize * 2', 'm_differencesSize * 4')
            text = text.replace('m_differencesSize*2', 'm_differencesSize*4')
            # The upstream comment omits the delta array element multiplier.
            text = text.replace('to_string(m_differencesSize)\n                  + ")+"',
                                'to_string(m_differencesSize)\n                  + "*4)+"')
            path.write_text(text)
        build = temporary / 'build'
        generated = temporary / 'generated'
        generated.mkdir()
        commands = [
            [cmake, '-S', str(source), '-B', str(build), '-DBUILD_GENERATOR=ON',
             '-DBUILD_TESTING=OFF', '-DCMAKE_BUILD_TYPE=Release'],
            [cmake, '--build', str(build), '--target', 'Generator', '--config', 'Release', '--parallel', '4'],
        ]
        with (temporary / 'generator.log').open('w+') as log:
            for command in commands:
                result = subprocess.run(command, stdin=subprocess.DEVNULL, stdout=log, stderr=log)
                if result.returncode:
                    log.seek(0)
                    raise RuntimeError(log.read())
            # Visual Studio places configuration-specific executables below Release.
            generator = next((p for p in (build / 'Generator', build / 'Generator.exe',
                                         build / 'Release/Generator.exe') if p.is_file()), None)
            if generator is None:
                raise RuntimeError('The generator executable is missing')
            subprocess.run([str(generator), str(DATA), str(generated)], check=True,
                           stdin=subprocess.DEVNULL, stdout=log, stderr=log)
        for name, expected in manifest['outputs'].items():
            path = generated / name
            if check:
                if digest(path) != expected or path.read_bytes() != (OUTPUT / name).read_bytes():
                    raise ValueError('Generated table differs: ' + name)
            else:
                (OUTPUT / name).write_bytes(path.read_bytes())
                manifest['outputs'][name] = digest(path)
            print(name, digest(path))
    if not check:
        (OUTPUT / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--cmake', default='cmake')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    reproduce(args.archive.resolve(), args.cmake, args.check)
