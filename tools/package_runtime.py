#!/usr/bin/env python3
"""Inspect the extracted package; no application runtime dependency on Python."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


def tool_path():
    if sys.platform == 'darwin':
        return shutil.which('otool')
    if sys.platform != 'win32':
        return shutil.which('objdump')
    direct = shutil.which('dumpbin')
    if direct:
        return direct
    installer = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
    if installer.is_file():
        result = subprocess.run([str(installer), '-latest', '-products', '*', '-requires',
                                 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-find',
                                 r'VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe'],
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
        candidates = result.stdout.decode('utf-8-sig').splitlines()
        if candidates:
            return candidates[0]
    return None


def category(path, root, platform):
    path = Path(path)
    if path.resolve().is_relative_to(root.resolve()):
        return 'bundled'
    name = path.name.lower()
    if platform == 'win32' and re.match(r'^(vcruntime|msvcp|concrt|vccorlib|mfc|mfcm)[0-9]', name):
        return 'external-visual-cpp-runtime'
    normalized = path.as_posix().lower()
    if platform == 'darwin' and normalized.startswith(('/system/library/', '/usr/lib/')):
        return 'os-runtime'
    if platform == 'win32':
        system = Path(os.environ.get('SystemRoot', r'C:\Windows')).as_posix().lower().rstrip('/') + '/'
        if normalized.startswith(system):
            return 'os-runtime'
    elif platform.startswith('linux') and normalized.startswith(('/lib/', '/lib64/', '/usr/lib/', '/usr/lib64/')):
        return 'system-desktop-runtime'
    return 'external-runtime'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    root = Path(args.root).resolve()
    tool = tool_path()
    cmake = shutil.which('cmake')
    if not tool or not cmake:
        raise RuntimeError('Native dependency inspector and CMake are required for package verification')
    names = ['secondbrain.exe', 'secondbrain-cli.exe'] if sys.platform == 'win32' else ['secondbrain', 'secondbrain-cli']
    programs = [root / name for name in names]
    if sys.platform == 'darwin':
        programs[0] = root / 'secondbrain.app/Contents/MacOS/secondbrain'
    raw = {'resolved': [], 'unresolved': [], 'conflicts': [], 'excluded_os_contract_patterns': [],
           'os_recursion_boundary_patterns': [], 'retained_redistributable_patterns': []}
    program_scans = {}
    with tempfile.TemporaryDirectory(prefix='SecondBrain runtime ü ') as temporary:
        for number, path in enumerate(programs):
            result_path = Path(temporary) / (str(number) + '.json')
            subprocess.run([cmake, '-DSB_PACKAGE_ROOT=' + root.as_posix(),
                            '-DSB_RUNTIME_BINARY=' + path.as_posix(),
                            '-DSB_RUNTIME_TOOL=' + Path(tool).as_posix(),
                            '-DSB_OUTPUT=' + result_path.as_posix(), '-P',
                            str(Path(__file__).with_suffix('.cmake').resolve())], check=True)
            scan = json.loads(result_path.read_text(encoding='utf-8'))
            program_scans[path.relative_to(root).as_posix()] = scan
            for key in raw:
                raw[key] = sorted(set(raw[key] + scan[key]))
    report = dict(raw, format=1, platform=sys.platform, inspector=tool,
                  program_scans=program_scans,
                  package_programs={}, declared_system_imports={}, binary_os_metadata={}, dependencies=[], limitations=[
                      'Linked imports only; optional libraries loaded dynamically need separate verification',
                      'System desktop availability and minimum OS versions need native clean-machine tests'])
    for path in programs:
        report['package_programs'][path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
        if sys.platform == 'darwin':
            listing = subprocess.check_output([tool, '-L', str(path)]).decode('utf-8')
            imports = [line.strip().split(' (compatibility version', 1)[0] for line in listing.splitlines()[1:]]
            report['declared_system_imports'][path.relative_to(root).as_posix()] = [value for value in imports if value.startswith(('/System/Library/', '/usr/lib/'))]
            commands = subprocess.check_output([tool, '-l', str(path)]).decode('utf-8')
            minimum = re.search(r'cmd LC_BUILD_VERSION\s+cmdsize \d+\s+platform (\d+)\s+minos ([\d.]+)\s+sdk ([\d.]+)', commands)
            if minimum:
                report['binary_os_metadata'][path.relative_to(root).as_posix()] = {
                    'macho_platform': minimum.group(1), 'minimum_os': minimum.group(2), 'sdk': minimum.group(3)}
    for value in raw['resolved']:
        report['dependencies'].append({'path': value, 'category': category(value, root, sys.platform)})
    problems = raw['unresolved'] + raw['conflicts'] + [d['path'] for d in report['dependencies'] if d['category'].startswith('external-')]
    report['problems'] = problems
    report['portable_linked_dependencies'] = not problems
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print('Package linked dependencies:', len(report['dependencies']), 'resolved;', len(problems), 'unresolved/external/conflicting')
    if problems:
        detail = ', '.join(problems).replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
        print('::error title=Package runtime dependencies::' + detail, flush=True)
        raise RuntimeError('Package requires unresolved external runtime libraries: ' + ', '.join(problems))


if __name__ == '__main__':
    try:
        main()
    except Exception as error:
        detail = (type(error).__name__ + ': ' + str(error)).replace('%', '%25').replace('\r', '%0D').replace('\n', '%0A')
        print('::error title=Package runtime inspection::' + detail, flush=True)
        raise
