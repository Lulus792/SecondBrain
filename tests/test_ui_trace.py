"""Milestones must use the drawn note and retain unfinished requests."""
import csv
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('trace_analyzer', ROOT / 'tools/analyze_ui_trace.py')
analyzer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analyzer)


class TraceTests(unittest.TestCase):
    def fixture(self):
        rows = []
        for phase in ('idle', 'scroll', 'camera', 'switch'):
            for frame in range(68):
                row = dict(phase=phase, frame=str(frame), request_serial='1', active_request='a.md',
                           request_elapsed_ms=str(len(rows) + 1), present_ok='1', reader_present='1',
                           reader_pending='0', reader_text_commands='1', reader_image_commands='0',
                           transitioning='0', transition_progress='1', presented_note='a.md', note_after='a.md',
                           input_ms='1', layout_ms='1', draw_ms='1', present_ms='1', apply_ms='1', observe_ms='1', total_ms='6')
                if phase == 'switch':
                    row.update(request_serial='2', active_request='b.md', request_elapsed_ms=str(frame + 1), note_after='b.md')
                    if frame >= 2:
                        row.update(presented_note='b.md', reader_pending='1')
                    if frame >= 4:
                        row.update(reader_pending='0', transitioning='1', transition_progress='0.5')
                    if frame >= 7:
                        row.update(transitioning='0', transition_progress='1')
                rows.append(row)
        return rows

    def inspect(self, rows):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'trace.csv'
            with path.open('w', newline='', encoding='utf-8') as stream:
                writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
                writer.writeheader()
                writer.writerows(rows)
            return analyzer.analyze(path)

    def test_changed_model_and_loading_do_not_count_as_drawn_text(self):
        result = self.inspect(self.fixture())
        self.assertEqual(result['requests'][1]['first_prepared_submission_ms'], 5)
        self.assertEqual(result['requests'][1]['first_settled_submission_ms'], 8)
        self.assertEqual(result['model_path_changed_after_present'], 2)

    def test_retarget_before_ready_is_reported_without_an_invented_latency(self):
        rows = self.fixture()
        for row in rows[-68:]:
            row.update(reader_pending='1')
        request = self.inspect(rows)['requests'][1]
        self.assertEqual(request['status'], 'not_prepared_in_sample')
        self.assertIsNone(request['first_prepared_submission_ms'])
        self.assertIsNone(request['first_settled_submission_ms'])

    def test_bad_timing_and_presentation_are_rejected(self):
        for key, value in [('present_ok', '0'), ('draw_ms', 'nan'), ('request_elapsed_ms', 'inf'),
                           ('total_ms', '100'), ('reader_pending', 'maybe')]:
            rows = self.fixture()
            rows[5][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.inspect(rows)


if __name__ == '__main__':
    unittest.main()
