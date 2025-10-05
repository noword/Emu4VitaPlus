#!/usr/bin/env python
from trans import Translation
from language import *

LANGUAGE_ENUM = []
NAMES = []

lang_trans = Translation('language.json')
languages = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), lang_trans[0].keys()))

for language in languages:
    LANGUAGE_ENUM.append(f'    LANGUAGE_{language.upper()},')
LANGUAGE_ENUM.append('    LANGUAGE_COUNT,')

for language in languages:
    NAMES.append(f'    "{language}", ')

LANGUAGE_ENUM = '\n'.join(LANGUAGE_ENUM)
NAMES = '\n'.join(NAMES)

# Generate language_define.h
open('language_define.h', 'w', encoding='utf-8').write(
    f'''#pragma once

enum LANGUAGE{{
{LANGUAGE_ENUM}
}};

extern const char *gLanguageNames[];

'''
)

# Generate language_define.cpp
open('language_define.cpp', 'w', encoding='utf-8').write(
    f'''#include "language_define.h"

const char *gLanguageNames[] = {{
{NAMES}
}};
'''
)


for NAME, TAG, USE_ENGLISH_WHEN_EMPTY in (('language', 'Tag', True), ('translation', 'English', False)):
    trans = Translation(f'{NAME}.json').get_trans(index=TAG)
    zlangauges = ZLanguages()
    for lang in LANGS:
        zlangauges[lang] = Texts()

    for k, v in trans.items():
        for lang in LANGS:
            s = v[lang]
            if len(s) == 0 and USE_ENGLISH_WHEN_EMPTY:
                s = v['English']
            zlangauges[lang].append(s)
    zlangauges.save(MyFile(f'{NAME}.zbin', 'wb'))
