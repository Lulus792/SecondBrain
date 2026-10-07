"""Real compiled package dependency fixture: bundled and omitted library."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import importlib.util

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('package_inspector', ROOT / 'tools/package_runtime.py')
inspector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inspector)
PROJECT = '''cmake_minimum_required(VERSION 3.24)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
project(RuntimeFixture LANGUAGES C)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
add_library(sb_runtime_fixture SHARED library.c)
set_target_properties(sb_runtime_fixture PROPERTIES INSTALL_NAME_DIR "@loader_path" BUILD_WITH_INSTALL_NAME_DIR TRUE)
add_executable(runtime_fixture main.c)
target_link_libraries(runtime_fixture PRIVATE sb_runtime_fixture)
'''


@unittest.skipUnless(shutil.which('cmake'), 'CMake unavailable')
class PackageRuntimeTests(unittest.TestCase):
    @unittest.skipUnless(sys.platform == 'win32', 'Native Windows path classification')
    def test_visual_cpp_runtime_in_system_directory_requires_separate_distribution(self):
        import os
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            system = Path(os.environ['SystemRoot']) / 'System32'
            self.assertEqual(inspector.category(system / 'kernel32.dll', root, 'win32'), 'os-runtime')
            self.assertEqual(inspector.category(system / 'vcruntime140.dll', root, 'win32'), 'external-visual-cpp-runtime')
            self.assertEqual(inspector.category(root / 'vcruntime140.dll', root, 'win32'), 'bundled')

    def test_bundled_library_is_accepted_and_omission_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix='SecondBrain package ü ') as temporary:
            root = Path(temporary)
            source = root / 'source'
            source.mkdir()
            (source / 'CMakeLists.txt').write_text(PROJECT, encoding='utf-8')
            (source / 'library.c').write_text('#ifdef _WIN32\n__declspec(dllexport)\n#endif\nint fixture(void) { return 7; }\n', encoding='utf-8')
            (source / 'main.c').write_text('int fixture(void); int main(void) { return fixture() != 7; }\n', encoding='utf-8')
            build = root / 'build'
            for command in [['cmake', '-S', str(source), '-B', str(build), '-DCMAKE_BUILD_TYPE=Release'],
                            ['cmake', '--build', str(build), '--config', 'Release']]:
                result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                self.assertEqual(result.returncode, 0, result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace'))
            executable = next(build.rglob('runtime_fixture.exe' if sys.platform == 'win32' else 'runtime_fixture'))
            pattern = 'sb_runtime_fixture.dll' if sys.platform == 'win32' else 'libsb_runtime_fixture.dylib' if sys.platform == 'darwin' else 'libsb_runtime_fixture.so'
            library = next(build.rglob(pattern))
            package = root / 'package'
            folder = package / 'secondbrain.app/Contents/MacOS' if sys.platform == 'darwin' else package
            folder.mkdir(parents=True)
            shutil.copyfile(executable, folder / ('secondbrain.exe' if sys.platform == 'win32' else 'secondbrain'))
            shutil.copyfile(executable, package / ('secondbrain-cli.exe' if sys.platform == 'win32' else 'secondbrain-cli'))
            if sys.platform == 'darwin':
                # CLI and bundle executable each use @loader_path.
                shutil.copyfile(library, package / library.name)
            shutil.copyfile(library, folder / library.name)
            output = root / 'report.json'
            command = [sys.executable, str(ROOT / 'tools/package_runtime.py'), '--root', str(package), '--output', str(output)]
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(result.returncode, 0, result.stdout.decode(errors='replace') + result.stderr.decode(errors='replace'))
            data = json.loads(output.read_text(encoding='utf-8'))
            self.assertTrue(any(d['category'] == 'bundled' and 'sb_runtime_fixture' in d['path'] for d in data['dependencies']))
            for path in package.rglob(library.name): path.unlink()
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertNotEqual(result.returncode, 0)
            data = json.loads(output.read_text(encoding='utf-8'))
            self.assertFalse(data['portable_linked_dependencies'])
            self.assertTrue(data['unresolved'] or any(d['category'].startswith('external-') for d in data['dependencies']))
            self.assertTrue(data['problems'])
            self.assertIn(b'::error title=Package runtime dependencies::', result.stdout)


if __name__ == '__main__':
    unittest.main()
