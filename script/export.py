#!/usr/bin/env python
from trans import Translation

lang_trans = Translation('language.json').get_trans(index='Tag')
trans_trans = Translation('translation.json').get_trans(index='English')
arch_trans = Translation('arch.json').get_trans(index='Tag')

languages = None

TEXT_ENUM = []
LANGUAGE_ENUM = []
ARCH_ENUM = []
NAMES = []
TEXTS = []
TRANS = []
ARCHS = []

for k, v in lang_trans.items():
    attr = f'{k},'
    TEXT_ENUM.append(f'    {attr:20s} // "{v["English"]}"')
    if languages is None:
        languages = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), v.keys()))
TEXT_ENUM.append('    TEXT_COUNT,')

for i, language in enumerate(languages):
    LANGUAGE_ENUM.append(f'    LANGUAGE_{language.upper()},')
LANGUAGE_ENUM.append('    LANGUAGE_COUNT,')

ARCH_ENUM = list(arch_trans.keys())
ARCH_ENUM.append('ARCH_COUNT,')

for language in languages:
    T = []
    for k, v in lang_trans.items():
        s = f'"{v[language]}",'
        T.append(f'    {s:30s} // {k}')
    T = '\n'.join(T)
    TEXTS.append(
        f'''// {language}
{{
{T}
}},
'''
    )

for language in languages:
    NAMES.append(f'    "{language}", ')
NAMES = '\n'.join(NAMES)

for k, v in trans_trans.items():
    t = []
    for language in languages[1:]:
        t.append(f'"{v[language]}"')
    t = ',\n     '.join(t)
    TRANS.append(
        f'''    {{"{k}", 
    {{{t}}},
    }}'''
    )

for language in languages:
    T = []
    for k, v in arch_trans.items():
        lang = v[language].replace('"', '\\"')
        s = f'"{lang}",'
        T.append(f'    // {k}')
        T.append(f'    {s}')
    T = '\n'.join(T)
    ARCHS.append(
        f'''// {language}
{{
{T}
}},
'''
    )

TEXT_ENUM = '\n'.join(TEXT_ENUM)
LANGUAGE_ENUM = '\n'.join(LANGUAGE_ENUM)
ARCH_ENUM = ',\n'.join(ARCH_ENUM)
TEXTS = '\n'.join(TEXTS)
TRANS = ',\n'.join(TRANS)
ARCHS = '\n'.join(ARCHS)

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

# Generate language_frontend.h
open('language_frontend.h', 'w', encoding='utf-8').write(
    f'''#pragma once
#include <unordered_map>
#include <array>
#include <string>
#include "language_define.h"

enum TEXT_ENUM{{
{TEXT_ENUM}
}};

extern const char *gTexts[][TEXT_ENUM::TEXT_COUNT];
typedef std::array<const char *, LANGUAGE::LANGUAGE_COUNT - 1> TRANS;
extern std::unordered_map<std::string, TRANS> gTrans;
'''
)

# Generate language_frontend.cpp
open('language_frontend.cpp', 'w', encoding='utf-8').write(
    f'''#include "language_frontend.h"

const char *gTexts[][TEXT_ENUM::TEXT_COUNT] = {{
{TEXTS}
}};

std::unordered_map<std::string, TRANS> gTrans = {{
{TRANS}
}};
'''
)

# Generate language_arch.h
open('language_arch.h', 'w', encoding='utf-8').write(
    f'''#pragma once
#include "language_define.h"

enum ARCH_ENUM{{
{ARCH_ENUM}
}};

extern const char *gArchs[][ARCH_ENUM::TEXT_COUNT];
'''
)

# Generate language_arch.cpp
open('language_arch.cpp', 'w', encoding='utf-8').write(
    f'''#include "language_arch.h"

const char *gTexts[][ARCH_ENUM::ARCH_COUNT] = {{
{ARCHS}
}};
'''
)
