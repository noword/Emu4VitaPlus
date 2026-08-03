import json
from pathlib import Path
import re

LOG_NAME = 'Emu4Vita++.log'
REG_PREFIX = r'\[[DI]\] \d\d:\d\d:\d\d.\d\d\d\s+?'
OPTION_REGS = (
    REG_PREFIX + r'desc: (.*)',
    REG_PREFIX + r'info: (.*)',
    REG_PREFIX + r'label\d*: (.*)',
)

CORE_REGS = REG_PREFIX + r'core: (.*)'

data = {}
for name in Path('.').rglob(LOG_NAME):
    print(f'### {name}')
    buf = open(name, encoding='utf-8').read()
    m = re.search(CORE_REGS, buf)
    if m is None:
        continue
    core = m.group(1)
    data[core] = []
    print(f'core: {core}')
    for reg in OPTION_REGS:
        for m in re.findall(reg, buf):
            print(m)
            data[core].append(m)

json.dump(data, open('core_options.json', 'w', encoding='utf-8'), indent=4, ensure_ascii=False)
