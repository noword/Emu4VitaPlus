#!/usr/bin/env python
from trans import Translation

TEXT_ENUM = []

lang_trans = Translation('language.json').get_trans(index='Tag')
trans_trans = Translation('translation.json').get_trans(index='English')

for k, v in lang_trans.items():
    attr = f'{k},'
    TEXT_ENUM.append(f'    {attr:20s} // "{v["English"]}"')
TEXT_ENUM.append('    TEXT_COUNT,')

TEXT_ENUM = '\n'.join(TEXT_ENUM)
TRANS_COUNT = len(trans_trans)

# Generate language_frontend.h
open('language_frontend.h', 'w', encoding='utf-8').write(
    f'''#pragma once
#include "language_define.h"

enum TEXT_ENUM{{
{TEXT_ENUM}
}};


#define TRANSLATION_COUNT {TRANS_COUNT}
'''
)
