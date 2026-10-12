import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
spec = importlib.util.spec_from_file_location('windows_runtime', Path(__file__).resolve().parents[1] / 'tools/record_windows_runtime.py')
runtime = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runtime)


class WindowsRuntimeInventoryTests(unittest.TestCase):
    def repro_fixture(self, root):
        program = root/'accesskit.dll'
        program.write_bytes(b'DLL fixture')
        map_path = root/'accesskit.map'
        map_path.write_text(' 0001:0000 __check 0000000180000000 f LIBCMT:gs_cookie.obj\n')
        Path(str(map_path)+'.identity').write_text(
            'SecondBrain link map 1\nprogram_sha256='+runtime.digest(program)+
            '\nmap_sha256='+runtime.digest(map_path)+'\n')
        sdk = root/'SDK mit ü'
        sdk.mkdir()
        library = sdk/'libcmt.lib'
        library.write_bytes(b'original SDK archive bytes')
        repro = root/'link repro ü'
        repro.mkdir()
        (repro/'LIBCMT.lib').write_bytes(library.read_bytes())
        records = {str(library): {'library': 'libcmt', 'sha256': runtime.digest(library)}}
        return program, map_path, repro, records

    def test_dll_repro_matches_captured_bytes_to_sdk_inventory(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            record = runtime.dll_repro(*args)
            self.assertEqual(record['sdk_libraries'][0]['named_object_members'], ['gs_cookie.obj'])
            self.assertEqual(record['sdk_libraries'][0]['byte_identical_sdk_file'], next(iter(args[3])))

    def test_dll_repro_rejects_missing_capture(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            (args[2]/'LIBCMT.lib').unlink()
            with self.assertRaisesRegex(ValueError, 'Expected one captured'):
                runtime.dll_repro(*args)

    def test_dll_repro_rejects_different_sdk_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            (args[2]/'LIBCMT.lib').write_bytes(b'other toolchain')
            with self.assertRaisesRegex(ValueError, 'matching SDK inventory'):
                runtime.dll_repro(*args)

    def test_dll_repro_rejects_changed_sdk_original(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            Path(next(iter(args[3]))).write_bytes(b'changed installed SDK')
            with self.assertRaisesRegex(ValueError, 'changed during DLL inventory'):
                runtime.dll_repro(*args)

    def test_dll_repro_rejects_ambiguous_capture(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            other = args[2]/'other'
            other.mkdir()
            (other/'LIBCMT.lib').write_bytes(b'original SDK archive bytes')
            with self.assertRaisesRegex(ValueError, 'Expected one captured'):
                runtime.dll_repro(*args)

    def test_dll_repro_rejects_changed_dll_identity(self):
        with tempfile.TemporaryDirectory() as temporary:
            args = self.repro_fixture(Path(temporary))
            args[0].write_bytes(b'changed DLL')
            with self.assertRaisesRegex(ValueError, 'no longer matches'):
                runtime.dll_repro(*args)

    def test_search_alone_does_not_prove_loading(self):
        with self.assertRaises(ValueError):
            runtime.loaded_libraries('Searching C:\\SDK\\LIBCMT.lib:\n')

    def test_unambiguous_basename_and_direct_path(self):
        text = ('  3>Searching C:\\SDK mit ü\\LIBCMT.lib:\n'
                '  3>Loaded LIBCMT.lib(gs_cookie.obj)\n'
                '  2>Loaded C:\\Kits\\libucrt.lib(printf.obj)\n')
        found = runtime.loaded_libraries(text)
        self.assertEqual(found['libcmt']['C:\\SDK mit ü\\LIBCMT.lib'], {'gs_cookie.obj'})
        self.assertEqual(found['libucrt']['C:\\Kits\\libucrt.lib'], {'printf.obj'})

    def test_ambiguous_search_paths_fail_closed(self):
        text = ('Searching C:\\Old\\LIBCMT.lib:\nSearching C:\\New\\LIBCMT.lib:\n'
                'Loaded LIBCMT.lib(gs_cookie.obj)\n')
        with self.assertRaises(ValueError):
            runtime.loaded_libraries(text)

    def test_map_members_do_not_count_imports_or_source_symbols(self):
        text = (' 0001:0000 __check 0000000140000000 f LIBCMT:gs_cookie.obj\n'
                ' 0001:0100 __write 0000000140000100 f libucrt:write.obj\n'
                ' 0001:0200 __imp_Open 0000000140000200 KERNEL32:KERNEL32.dll\n'
                ' 0001:0300 sb_open 0000000140000300 f sb_core:platform.obj\n')
        self.assertEqual(runtime.map_members(text), {'libcmt': {'gs_cookie.obj'}, 'libucrt': {'write.obj'}})


if __name__ == '__main__':
    unittest.main()
