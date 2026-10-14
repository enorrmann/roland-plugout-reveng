"""Patch parameter ids of the JX-3P 'patch' struct: id = 0x6CD + nibble_offset/2."""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../tools'))
import xml.etree.ElementTree as ET
import patches as P
ROOT=os.path.join(os.path.dirname(__file__), '../../..')
def patch_params(script=os.path.join(ROOT,'plugins/JX-3P/Script/Script.xml')):
    L=P.ScriptLayout(script)
    root=ET.parse(script).getroot()
    types={e.findtext('type'):e for e in root.findall('structType')}
    out=[]
    for leaf in L.leaves['patch']:
        t=leaf.path
        e=types.get(t) or types.get({'NAME0':'PAT_NAME'}.get(t,t))
        if e is None or t.startswith('NAME') or t.startswith('PAT_NAME'): continue
        off=leaf.offset
        for v in e.findall('value'):
            n=int(v.findtext('type')[3])
            out.append((0x6CD+off//2, v.findtext('name'), int(v.findtext('default') or 0)))
            off+=n
    return out
if __name__=='__main__':
    for i,n,d in patch_params(): print(hex(i),n,d)
