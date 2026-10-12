#!/usr/bin/env python3
"""Sign a new package copy with native tools; never publish or change inputs."""
import argparse
import hashlib
import json
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import sys
from urllib.parse import urlparse


def snapshot(root):
    if root.is_symlink() or not root.is_dir():
        raise ValueError('A real extracted package directory is required')
    files = {}
    for path in sorted(root.rglob('*')):
        if path.is_symlink():
            raise ValueError('Package symlinks are not supported by this signing workflow')
        if path.is_file():
            files[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    return files


def validate_package(source, platform):
    common = {'LICENSE', 'QUICKSTART.txt', 'licenses'}
    if platform == 'macos':
        allowed = common | {'secondbrain.app', 'secondbrain-cli'}
        programs = ['secondbrain.app/Contents/MacOS/secondbrain', 'secondbrain-cli']
        with (source/'secondbrain.app/Contents/Info.plist').open('rb') as stream:
            info = plistlib.load(stream)
        if info.get('CFBundleIdentifier') != 'org.secondbrain.desktop':
            raise ValueError('Unexpected application bundle identifier')
    else:
        allowed = common | {'assets', 'secondbrain.exe', 'secondbrain-cli.exe', 'accesskit.dll'}
        programs = ['secondbrain.exe', 'secondbrain-cli.exe', 'accesskit.dll']
    if {p.name for p in source.iterdir()} != allowed:
        raise ValueError('Package root must contain exactly the known distribution entries')
    if any(not (source/p).is_file() for p in programs):
        raise ValueError('Package program missing')
    return programs


def settings(platform, identity, team, certificate, timestamp, adhoc):
    if platform == 'macos':
        if certificate or timestamp:
            raise ValueError('Windows signing options are invalid for macOS')
        if adhoc:
            if identity or team:
                raise ValueError('Ad-hoc tests cannot claim a publisher identity')
        elif not identity or identity == '-' or not re.fullmatch(r'[A-Z0-9]{10}', team or ''):
            raise ValueError('Developer ID identity and ten-character Team ID are required')
    else:
        if adhoc or identity or team:
            raise ValueError('macOS signing options are invalid for Windows')
        if not re.fullmatch(r'[0-9a-fA-F]{40}', certificate or ''):
            raise ValueError('A certificate SHA1 thumbprint from the certificate store is required')
        parsed = urlparse(timestamp or '')
        if parsed.scheme != 'https' or not parsed.hostname or parsed.username or parsed.password or parsed.fragment:
            raise ValueError('An HTTPS RFC3161 timestamp URL without credentials is required')


def native_tool(name):
    path = shutil.which(name)
    if not path:
        raise RuntimeError('Native signing tool not found: '+name)
    return path


def command(args):
    return subprocess.run(args, check=True, capture_output=True, timeout=120)


def sign(source, output, platform, identity=None, team=None, certificate=None,
         timestamp=None, adhoc=False, runner=command):
    settings(platform, identity, team, certificate, timestamp, adhoc)
    if (platform == 'macos' and sys.platform != 'darwin') or (platform == 'windows' and sys.platform != 'win32'):
        raise ValueError('Native signing must run on its target operating system')
    if source.is_symlink() or output.is_symlink():
        raise ValueError('Source and output must not be symlinks')
    source, output = source.resolve(strict=True), output.resolve()
    if output.exists() or output.is_relative_to(source) or source.is_relative_to(output):
        raise ValueError('Choose a new output outside the source package')
    before = snapshot(source)
    programs = validate_package(source, platform)
    tool = native_tool('codesign' if platform == 'macos' else 'signtool')
    output.mkdir(parents=True, exist_ok=False)
    stage = output/'SecondBrain'
    try:
        shutil.copytree(source, stage)
        if snapshot(stage) != before:
            raise RuntimeError('Package changed while preparing the signing copy')
        if platform == 'macos':
            sign_flags = ['--force', '--sign', '-' if adhoc else identity, '--options', 'runtime',
                          '--timestamp=none' if adhoc else '--timestamp']
            runner([tool, *sign_flags, '--identifier', 'org.secondbrain.cli', str(stage/'secondbrain-cli')])
            runner([tool, *sign_flags, str(stage/'secondbrain.app')])
            for target in ('secondbrain-cli', 'secondbrain.app'):
                args = [tool, '--verify', '--strict', '--deep']
                if not adhoc:
                    identifier = 'org.secondbrain.cli' if target == 'secondbrain-cli' else 'org.secondbrain.desktop'
                    requirement = ('anchor apple generic and identifier "'+identifier+
                                   '" and certificate 1[field.1.2.840.113635.100.6.2.6] exists'
                                   ' and certificate leaf[subject.OU] = "'+team+
                                   '" and certificate leaf[field.1.2.840.113635.100.6.1.13] exists')
                    args += ['-R', '= '+requirement]
                runner([*args, str(stage/target)])
                details = runner([tool, '--display', '--verbose=2', str(stage/target)]).stderr.decode('utf-8')
                if not re.search(r'flags=0x[0-9a-fA-F]+\([^\n]*runtime', details):
                    raise RuntimeError('Hardened runtime is missing')
                if not adhoc and not re.search(r'^Timestamp=.+$', details, re.M):
                    raise RuntimeError('Secure signing timestamp is missing')
        else:
            for target in programs:
                runner([tool, 'sign', '/sha1', certificate, '/fd', 'SHA256',
                        '/tr', timestamp, '/td', 'SHA256', str(stage/target)])
                runner([tool, 'verify', '/pa', '/all', '/tw', str(stage/target)])
        after = snapshot(stage)
        # Only program signatures and Apple's resource seal may change.
        for name, digest in before.items():
            if name not in programs and name != 'secondbrain.app/Contents/_CodeSignature/CodeResources':
                if after.get(name) != digest:
                    raise RuntimeError('Signing changed a non-program package file: '+name)
        if snapshot(source) != before:
            raise RuntimeError('Original package changed during signing')
        receipt = {'format': 1, 'platform': platform,
                   'status': 'ad-hoc-test-only' if adhoc else 'signed',
                   'publisher_signature_verified': not adhoc,
                   'notarization_verified': False,
                   'release_ready': False,
                   'source_files_preserved': len(before), 'source_hashes': before,
                   'signed_hashes': after,
                   'scope': 'native signatures only; product, license and release approval remain separate'}
        (output/'signing.json').write_text(json.dumps(receipt, indent=2)+'\n', encoding='utf-8')
        return receipt
    except Exception:
        (output/'FAILED.txt').write_text('Signing did not complete. This directory is not a release package.\n')
        raise


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--platform', choices=['macos', 'windows'], required=True)
    parser.add_argument('--identity')
    parser.add_argument('--team-id')
    parser.add_argument('--certificate-sha1')
    parser.add_argument('--timestamp-url')
    parser.add_argument('--ad-hoc-test', action='store_true')
    args = parser.parse_args()
    receipt = sign(args.source, args.output, args.platform, args.identity, args.team_id,
                   args.certificate_sha1, args.timestamp_url, args.ad_hoc_test)
    print('Signing status:', receipt['status'], '; notarization:', receipt['notarization_verified'])
