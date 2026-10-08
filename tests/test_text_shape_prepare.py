"""Verify pinned SDL_ttf preparation and rejection without source mutation."""
from pathlib import Path
import hashlib
import subprocess
import sys
import tempfile

root=Path(__file__).resolve().parents[1]
marker=b'\n/* SecondBrain contextual glyph positions, UI only. */\n#include "ttf_shape.inc"\n'
prepared=Path(sys.argv[1]).read_bytes()
assert prepared.endswith(marker)
fixed=prepared[:-len(marker)]
guard=(root/'third_party/ui/ttf_empty_glyph_guard.inc').read_bytes()
assert fixed.count(guard)==1
original=fixed.replace(guard,b'',1)
assert hashlib.sha256(original).hexdigest()=='25a42804b18809e5c4b2eb8ed787701551d0c680aff774b7d8c54486c0d42d38'
assert hashlib.sha256(fixed).hexdigest()=='effab05fe248deadc02a1259ca640fa9c785909c3398ae16b13a1fabbe9c1b9e'
with tempfile.TemporaryDirectory(prefix='secondbrain-ttf-') as temp:
 for name,data,accepted in [('original',original,True),('old-prepared',original+marker,True),('fixed-prepared',prepared,True),('changed',original+b'/* unknown */\n'+marker,False),('truncated',b'bad'+marker,False)]:
  folder=Path(temp)/name;source=folder/'src/SDL_ttf.c';source.parent.mkdir(parents=True);source.write_bytes(data)
  command=[sys.argv[2],f'-Dsb_sdl_ttf_SOURCE_DIR={folder}','-P',str(root/'cmake/PrepareTextShape.cmake')]
  result=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  assert (result.returncode==0)==accepted,(name,result.stdout,result.stderr)
  assert source.read_bytes()==(prepared if accepted else data),name
  if accepted:
   assert subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE).returncode==0
   assert source.read_bytes()==prepared
print('5 pinned SDL_ttf source cases passed: original, upgrade, idempotence and unchanged rejection.')
