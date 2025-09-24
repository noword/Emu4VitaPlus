#!/usr/bin/env python
from trans import Translation

LANGS = None
TEXT_ENUM = []
TEXTS = []
TRANS = []

lang_trans = Translation('language.json').get_trans(index='Tag')
trans_trans = Translation('translation.json').get_trans(index='English')

for k, v in lang_trans.items():
    attr = f'{k},'
    TEXT_ENUM.append(f'    {attr:20s} // "{v["English"]}"')
    if LANGS is None:
        LANGS = list(filter(lambda x: x not in ('No.', 'Tag', 'Comments'), v.keys()))
TEXT_ENUM.append('    TEXT_COUNT,')

for lang in LANGS:
    T = []
    for k, v in lang_trans.items():
        s = f'"{v[lang]}",'
        T.append(f'    {s:30s} // {k}')
    T = '\n'.join(T)
    TEXTS.append(
        f'''// {lang}
{{
{T}
}},
'''
    )

for k, v in trans_trans.items():
    k = k.replace('"', '\\"')
    t = [f'     "{k}"']
    for language in LANGS[1:]:
        lang = v[language].replace('"', '\\"')
        t.append(f'"{lang}"')
    t = ',\n     '.join(t)
    TRANS.append(
        f'''{{
{t},
}}'''
    )

TRANS_COUNT = len(TRANS)
TEXT_ENUM = '\n'.join(TEXT_ENUM)
TEXTS = '\n'.join(TEXTS)
TRANS = ',\n'.join(TRANS)

# Generate language_frontend.h
open('language_frontend.h', 'w', encoding='utf-8').write(
    f'''#pragma once
#include "language_define.h"

enum TEXT_ENUM{{
{TEXT_ENUM}
}};

#define TRANSLATION_COUNT {TRANS_COUNT}

extern const char *gTexts[][TEXT_ENUM::TEXT_COUNT];
extern const char *gTrans[][LANGUAGE_COUNT];
'''
)

# Generate language_frontend.cpp
open('language_frontend.cpp', 'w', encoding='utf-8').write(
    f'''#include "language_frontend.h"

const char *gTexts[][TEXT_ENUM::TEXT_COUNT] = {{
{TEXTS}
}};

const char *gTrans[][LANGUAGE_COUNT] = {{
{TRANS}
}};
'''
)
