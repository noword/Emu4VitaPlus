import json
from pathlib import Path
import re

LOG_NAME = 'Emu4Vita++.log'

REG_PREFIX = r'\[[DI]\] \d\d:\d\d:\d\d.\d\d\d\s+?'

OPTION_REGS = (
    REG_PREFIX + r'desc: (.+)',
    REG_PREFIX + r'info: (.+)',
    REG_PREFIX + r'label\d*: (.+)',
)

VALUE_REG = REG_PREFIX + r'value: (.+?);\s*(.*)'
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
            if reg.startswith(REG_PREFIX + 'desc:'):
                for s in m.split(' > '):
                    data[core].append(s)
            else:
                data[core].append(m)
    for v in re.findall(VALUE_REG, buf):
        print(v)
        data[core].append(v[0])
        for v1 in v[1].split('|'):
            data[core].append(v1)

for k, v in data.items():
    data[k] = list(v)
json.dump(data, open('core_options.json', 'w', encoding='utf-8'), indent=4, ensure_ascii=False)
