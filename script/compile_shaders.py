#!/usr/bin/env python
import os
from pathlib import Path

for name in Path('shaders').rglob('*.cg'):
    print(name)
    stem = name.stem
    os.system(f'psp2cgc.exe -profile sce_vp_psp2 -fastprecision -DVERTEX {name} -o {stem}_v.gxp -sdb {stem}_v.sdb')
    os.system(f'psp2cgc.exe -profile sce_fp_psp2 -fastprecision -DFRAGMENT {name} -o {stem}_f.gxp -sdb {stem}_f.sdb')
