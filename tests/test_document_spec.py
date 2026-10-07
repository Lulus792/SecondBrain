"""Independent original HTML tree oracle for the own C container tree."""
from html.parser import HTMLParser
from pathlib import Path
import json
import subprocess
import sys
import tempfile

class Node:
    def __init__(self, kind, level=0, ordered=False, start=0, tight=True):
        self.kind, self.level, self.ordered, self.start, self.tight = kind, level, ordered, start, tight
        self.text, self.children = '', []
    def value(self):
        text = self.text if self.kind in ('code', 'raw') else self.text.strip(' \t\r\n').replace('\n', ' ')
        return (self.kind, self.level if self.kind == 'heading' else 0,
                self.ordered if self.kind == 'list' else False, self.start if self.kind == 'list' and self.ordered else 0,
                self.tight if self.kind == 'list' else True,text,[c.value() for c in self.children])
class Expected(HTMLParser):
    def __init__(self):
        super().__init__(); self.root=Node('root'); self.stack=[self.root]; self.inline=0
    def implicit_end(self):
        if self.stack[-1].kind == 'p' and getattr(self.stack[-1], 'implicit', False): self.stack.pop()
    def handle_starttag(self, tag, attrs):
        attrs=dict(attrs)
        if tag in ('blockquote','ul','ol','li','p','pre','hr') or tag in ('h1','h2','h3','h4','h5','h6'):
            self.implicit_end()
            kind={'blockquote':'quote','ul':'list','ol':'list','li':'item','p':'p','pre':'code','hr':'rule'}.get(tag,'heading')
            n=Node(kind, int(tag[1]) if kind=='heading' else 0, tag=='ol',int(attrs.get('start','1')) if tag=='ol' else 0)
            self.stack[-1].children.append(n)
            if tag=='p' and self.stack[-1].kind=='item':
                for parent in reversed(self.stack):
                    if parent.kind=='list': parent.tight=False;break
            if tag!='hr': self.stack.append(n)
        elif tag=='img': self.handle_data(attrs.get('alt',''))
        elif tag not in ('a','em','strong','code','br'):
            self.handle_data(self.get_starttag_text())  # Existing literal HTML display contract
    def handle_endtag(self, tag):
        if tag in ('blockquote','ul','ol','li','p','pre') or tag in ('h1','h2','h3','h4','h5','h6'):
            if tag in ('li','blockquote','ul','ol'): self.implicit_end()
            self.stack.pop()
    def handle_data(self, data):
        if self.stack[-1].kind in ('root','quote','list','item'):
            if not data.strip(' \t\r\n'): return
            n=Node('p');n.implicit=True;self.stack[-1].children.append(n);self.stack.append(n)
        self.stack[-1].text += data
    def handle_comment(self, data):
        self.implicit_end();n=Node('raw');n.text='<!--'+data+'-->\n';self.stack[-1].children.append(n)

def actual(probe,path):
    lines=subprocess.check_output([probe,str(path)],text=True).splitlines();stack=[];root=None
    for line in lines:
        if line.startswith('+ '):
            kind,level,ordered,start,tight=line[2:].split();n=Node(kind,int(level),bool(int(ordered)),int(start),bool(int(tight)))
            if stack:stack[-1].children.append(n)
            else:root=n
            stack.append(n)
        elif line.startswith('T '):stack[-1].text=bytes.fromhex(line[2:]).decode('utf-8')
        elif line=='-':stack.pop()
        else:raise ValueError(line)
    if stack or root is None:raise ValueError('Incomplete document tree')
    return root.value()
def main():
    probe,fixture=sys.argv[1:];cases=json.loads(Path(fixture).read_text(encoding='utf-8'));failures=[]
    with tempfile.TemporaryDirectory(prefix='secondbrain-container-spec-') as folder:
        path=Path(folder)/'input.md'
        for case in cases:
            path.write_text(case['markdown'],encoding='utf-8');expected=Expected();expected.feed(case['html']);got=actual(probe,path)
            if got!=expected.root.value():
                failures.append(case['example']);print('DOCUMENT FAIL',case['example'],repr(case['markdown']), '\nACTUAL',got,'\nEXPECTED',expected.root.value())
    if failures:raise SystemExit(f'{len(failures)}/{len(cases)} document fixtures failed: {failures}')
    print(f'{len(cases)} original container fixtures passed against the independent HTML tree oracle.')
if __name__=='__main__':main()
