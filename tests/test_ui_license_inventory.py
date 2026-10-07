"""Integrity and coverage checks for the reviewed UI notice inventory."""
import copy
import hashlib
import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK_SHA = '97ac8966d2fadbc325ef960d874d89b50877253bcdcffaec499645f18b04ef7d'


def validate(data, directory):
    if data.get('format') != 1 or data.get('accesskit_original_lock_sha256') != LOCK_SHA:
        raise ValueError('Unreviewed source lock')
    identities = set()
    counts = {'macos': 0, 'windows': 0, 'linux': 0}
    for component in data['components']:
        identity = (component['name'], component['version'])
        if identity in identities:
            raise ValueError('Duplicate component')
        identities.add(identity)
        if not component['declared_license'] or not component['notices']:
            raise ValueError('Missing license assignment')
        platforms = component['platforms']
        if not platforms or len(platforms) != len(set(platforms)) or set(platforms) - counts.keys():
            raise ValueError('Invalid platform assignment')
        for platform in platforms:
            counts[platform] += 1
        for notice in component['notices']:
            if not re.fullmatch('[0-9a-f]{64}', notice['sha256']) or not notice['source'].startswith('https://'):
                raise ValueError('Missing original notice origin')
    if len(identities) != 113 or counts != {'macos': 20, 'windows': 24, 'linux': 90}:
        raise ValueError('Changed reviewed target closure')
    for name, expected in data['bundles'].items():
        path = directory / name
        if path.name != name or hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise ValueError('Notice bundle missing or changed')
        text = path.read_text(encoding='utf-8')
        if '\x00' in text:
            raise ValueError('Invalid notice text')
    cargo_text = (directory / 'AccessKit-transitive.txt').read_text(encoding='utf-8')
    for component in data['components']:
        if component['name'] + '@' + component['version'] not in cargo_text:
            raise ValueError('Unrepresented component')
        for notice in component['notices']:
            if 'SHA-256: ' + notice['sha256'] not in cargo_text:
                raise ValueError('Original payload missing')
    unicode = next(c for c in data['components'] if c['name'] == 'unicode-ident')
    if not any('LICENSE-UNICODE' in n['source'] for n in unicode['notices']):
        raise ValueError('Required AND license missing')
    origins = {n['source'] for n in data['source_notices']}
    for suffix in ['src/video/yuv2rgb/LICENSE', 'src/hidapi/LICENSE-bsd.txt']:
        if not any(url.endswith(suffix) for url in origins):
            raise ValueError('Embedded BSD notice missing')
    extra_text = (directory / 'UI-source-notices.txt').read_text(encoding='utf-8')
    for notice in data['source_notices']:
        prefix = {'SDL3': 'https://github.com/libsdl-org/SDL/blob/release-3.2.30/',
                  'HarfBuzz': 'https://github.com/harfbuzz/harfbuzz/blob/564bf9818a18709776856533829c0c04950773d6/'}
        if not notice['source'].startswith(prefix[notice['component']]):
            raise ValueError('Wrong original source repository')
        if 'SHA-256: ' + notice['sha256'] not in extra_text:
            raise ValueError('Source notice payload missing')


class InventoryTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT / 'third_party/license-manifest.json').read_text(encoding='utf-8'))
        self.directory = ROOT / 'third_party/licenses'

    def test_reviewed_inventory_and_original_payloads(self):
        validate(self.data, self.directory)

    def test_missing_transitive_assignment_rejected(self):
        broken = copy.deepcopy(self.data)
        broken['components'].pop()
        with self.assertRaises(ValueError):
            validate(broken, self.directory)

    def test_duplicate_assignment_rejected(self):
        broken = copy.deepcopy(self.data)
        broken['components'].append(copy.deepcopy(broken['components'][0]))
        with self.assertRaises(ValueError):
            validate(broken, self.directory)

    def test_changed_original_lock_rejected(self):
        broken = copy.deepcopy(self.data)
        broken['accesskit_original_lock_sha256'] = '0' * 64
        with self.assertRaises(ValueError):
            validate(broken, self.directory)

    def test_changed_bundle_rejected(self):
        broken = copy.deepcopy(self.data)
        broken['bundles']['AccessKit-transitive.txt'] = '0' * 64
        with self.assertRaises(ValueError):
            validate(broken, self.directory)

    def test_lost_unicode_and_license_rejected(self):
        broken = copy.deepcopy(self.data)
        component = next(c for c in broken['components'] if c['name'] == 'unicode-ident')
        component['notices'] = [n for n in component['notices'] if 'LICENSE-UNICODE' not in n['source']]
        with self.assertRaises(ValueError):
            validate(broken, self.directory)

    def test_wrong_original_source_repository_rejected(self):
        broken = copy.deepcopy(self.data)
        notice = next(n for n in broken['source_notices'] if n['component'] == 'HarfBuzz')
        notice['source'] = notice['source'].replace('harfbuzz/harfbuzz', 'libsdl-org/harfbuzz')
        with self.assertRaises(ValueError):
            validate(broken, self.directory)


if __name__ == '__main__':
    unittest.main()
