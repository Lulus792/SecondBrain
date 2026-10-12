import importlib.util
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
spec = importlib.util.spec_from_file_location('windows_runtime', Path(__file__).resolve().parents[1] / 'tools/record_windows_runtime.py')
runtime = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runtime)


class WindowsRuntimeInventoryTests(unittest.TestCase):
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
