#!/usr/bin/env python
from trans import Translation
from language import *

CODES = {
    "English": "GBR",
    "Chinese": "CHN",
    "Japanese": "JPN",
    "Italian": "ITA",
    "French": "FRA",
    "Spanish": "ESP",
    "Russian": "RUS",
}

LANGUAGE_ENUM = []
NAMES = []
NAMES_WITH_ICON = []

lang_trans = Translation('language.json')
languages = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), lang_trans[0].keys()))

for language in languages:
    LANGUAGE_ENUM.append(f'    LANGUAGE_{language.upper()},')
LANGUAGE_ENUM.append('    LANGUAGE_COUNT,')

for language in languages:
    NAMES.append(f'    "{language}", ')

for language in languages:
    NAMES_WITH_ICON.append(f'    ICON_COUNTRY_{CODES[language]} " {language}",')

LANGUAGE_ENUM = '\n'.join(LANGUAGE_ENUM)
NAMES = '\n'.join(NAMES)
NAMES_WITH_ICON = '\n'.join(NAMES_WITH_ICON)

# Generate language_define.h
open('language_define.h', 'w', encoding='utf-8').write(f'''#pragma once

enum LANGUAGE{{
{LANGUAGE_ENUM}
}};

extern const char *gLanguageNames[];
extern const char *gLanguageNamesWithIcon[];
''')

# Generate language_define.cpp
open('language_define.cpp', 'w', encoding='utf-8').write(f'''#include "language_define.h"
#include "icons.h"

const char *gLanguageNames[] = {{
{NAMES}
}};

const char *gLanguageNamesWithIcon[] = {{
{NAMES_WITH_ICON}
}};
''')


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
