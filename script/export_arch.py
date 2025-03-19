#!/usr/bin/env python
from trans import Translation

ARCH_ENUM = []
ARCHS = []

arch_trans = Translation('arch.json').get_trans(index='Tag')

ARCH_ENUM = list(arch_trans.keys())
ARCH_ENUM.append('INTRO_COUNT')

LANGS = next(iter(arch_trans.values())).keys()
LANGS = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), LANGS))

for language in LANGS:
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

ARCH_ENUM = ',\n'.join(ARCH_ENUM)
ARCHS = '\n'.join(ARCHS)

# Generate language_arch.h
open('language_arch.h', 'w', encoding='utf-8').write(
    f'''#pragma once
#include "language_define.h"

enum ARCH_ENUM{{
{ARCH_ENUM}
}};

extern const char *gArchs[][ARCH_ENUM::INTRO_COUNT];
'''
)

# Generate language_arch.cpp
open('language_arch.cpp', 'w', encoding='utf-8').write(
    f'''#include "language_arch.h"

const char *gTexts[][ARCH_ENUM::INTRO_COUNT] = {{
{ARCHS}
}};
'''
)
