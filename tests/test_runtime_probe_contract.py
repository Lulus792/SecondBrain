"""Metadata transport differences must not hide missing native startup facts."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('runtime_probe', Path(__file__).with_name('test_runtime_probe.py'))
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class ProbeContractTests(unittest.TestCase):
    def setUp(self):
        self.version = 'SecondBrain 0.9.73\nBuild: example\nSystem: Windows / AMD64\nConfiguration: Release\n'
        self.report = {'format': 1, 'build': self.version, 'rendered_frames': 4,
                       'window_visible': True, 'scope': 'normal-renderer-start-after-four-frames',
                       'renderer': 'direct3d11', 'video_driver': 'windows', 'loaded_modules': ['C:\\Windows\\System32\\kernel32.dll']}

    def test_crlf_stdout_and_lf_json_describe_the_same_build(self):
        self.assertEqual(probe.report_errors(self.report, self.version.encode()), [])
        self.assertEqual(probe.report_errors(self.report, self.version.replace('\n', '\r\n').encode()), [])

    def test_changed_build_is_still_rejected_after_line_end_normalization(self):
        wrong = self.version.replace('Build: example', 'Build: changed').replace('\n', '\r\n').encode()
        self.assertEqual(probe.report_errors(self.report, wrong), ['build'])

    def test_hidden_window_missing_modules_and_incomplete_rendering_are_rejected(self):
        self.report.update(window_visible=False, loaded_modules=[], rendered_frames=3)
        self.assertEqual(set(probe.report_errors(self.report, self.version.encode())),
                         {'window_visible', 'loaded_modules', 'rendered_frames'})


if __name__ == '__main__':
    unittest.main()
