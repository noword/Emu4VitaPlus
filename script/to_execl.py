#!/usr/bin/env python
from trans import Translation

trans = Translation('language.json')
trans.save('language.xlsx', index='Tag')

trans = Translation('translation.json')
trans.save('translation.xlsx', index='English')

trans = Translation('arch.json')
trans.save('arch.xlsx', index='Tag')
