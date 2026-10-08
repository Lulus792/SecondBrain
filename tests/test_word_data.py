from pathlib import Path
import importlib.util
root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('word_data',root/'tools/make_word_data.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
assert (root/'src/word_data.inc').read_text(encoding='utf-8')==module.generate()
print('Pinned Unicode 18.0 word sources and generated tables match.')
