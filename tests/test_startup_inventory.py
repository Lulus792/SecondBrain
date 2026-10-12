import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('startup', Path(__file__).resolve().parents[1] / 'tools/record_linux_startup.py')
startup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(startup)


class StartupInventoryTests(unittest.TestCase):
    def test_only_nonempty_output_contributions_count(self):
        text = ('Discarded input sections\n .text 0x0 0x20 /discarded/crti.o\n'
                'Linker script and memory map\nLOAD /unused/crtn.o\n'
                ' .text 0x100 0x0 /empty/crtbeginS.o\n'
                ' .comment 0x0 0x20 /debug/crtendS.o\n'
                ' .init 0x100 0x17 /SDK mit ü/crti.o\n'
                ' .eh_frame\n  0x120 0x4 /real/crtendS.o\n')
        self.assertEqual(set(startup.startup_contributors(text)), {'/SDK mit ü/crti.o', '/real/crtendS.o'})

    def test_missing_map_or_load_only_is_not_evidence(self):
        for text in ('LOAD /a/crti.o', 'Linker script and memory map\nLOAD /a/crti.o\n'):
            with self.assertRaises(ValueError):
                startup.startup_contributors(text)

    def test_multiarch_package_source_and_exact_owner(self):
        path = Path('/usr/lib/x86_64-linux-gnu/Scrt1.o')
        def run(*args):
            if args[1] == '-S':
                return 'libc6-dev:amd64: ' + str(path) + '\n'
            return 'libc6-dev:amd64\t2.39-1\tglibc\t2.39-1\tamd64\n'
        result = startup.package_metadata(path, run)
        self.assertEqual(result['source_package'], 'glibc')
        self.assertEqual(result['binary_package'], 'libc6-dev:amd64')
        with self.assertRaises(ValueError):
            startup.package_metadata(path, lambda *args: 'another: /some/other/file\n')

    def test_stale_program_or_map_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            program, path = Path(folder)/'program', Path(folder)/'map'
            program.write_bytes(b'original program');path.write_bytes(b'original map')
            Path(str(path)+'.identity').write_text('SecondBrain link map 1\nprogram_sha256='+startup.digest(program)+'\nmap_sha256='+startup.digest(path)+'\n')
            startup.checked_map(program, path)
            program.write_bytes(b'changed program')
            with self.assertRaises(ValueError):
                startup.checked_map(program, path)


if __name__ == '__main__':
    unittest.main()
