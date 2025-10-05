#include "language_string.h"
#include "file.h"
#include "global.h"
#include "log.h"

#define LANGUAGE_ZBIN_PATH "app0:assets/language.zbin"
#define TRANSLATION__ZBIN_PATH "app0:assets/translation.zbin"

std::array<std::string, TEXT_COUNT> LanguageString::_texts;
std::unordered_map<std::string, std::string> LanguageString::_trans;
std::array<std::string, TEXT_COUNT> LanguageString::_english_texts;

const char *TEXT(size_t index)
{
    return index < TEXT_COUNT ? LanguageString::Texts()[index].c_str() : "Unknown";
}

const char *const LanguageString::Get() const
{
    if (_text_id != INVALID_TEXT_ENUM)
    {
        return TEXT(_text_id);
    }

    if (gConfig->language == LANGUAGE_ENGLISH)
    {
        return _string.c_str();
    }

    auto iter = _trans.find(_string);
    if (iter == _trans.end())
    {
        return _string.c_str();
    }
    else
    {
        return iter->second.c_str();
    }
}

const char *const LanguageString::GetOriginal() const
{
    if (_text_id != INVALID_TEXT_ENUM)
    {
        return _text_id < TEXT_COUNT ? _english_texts[_text_id].c_str() : "Unknown";
    }

    return _string.c_str();
}

void LanguageString::Init()
{
    _InitTexts();
    _InitTrans();
}

static void _GetPoints(uint32_t *p, uint32_t lang, char **strings, uint32_t **offsets, uint32_t *count)
{
    p += p[lang] / sizeof(uint32_t);
    uint32_t size = *p++;
    *strings = (char *)p;
    p += size / sizeof(uint32_t);
    *count = *p++;
    *offsets = p;
}

void LanguageString::_InitTexts()
{
    char *buf;
    File::ReadCompressedFile(LANGUAGE_ZBIN_PATH, (void **)&buf);

    char *strings;
    uint32_t *offsets;
    uint32_t count;
    _GetPoints((uint32_t *)buf, gConfig->language, &strings, &offsets, &count);
    if (count != TEXT_COUNT)
    {
        LogError("count != TEXT_COUNT: %x %d", count, TEXT_COUNT);
    }

    for (int i = 0; i < TEXT_COUNT; i++)
    {
        _texts[i] = strings + *offsets++;
    }

    _GetPoints((uint32_t *)buf, 0, &strings, &offsets, &count);
    if (count != TEXT_COUNT)
    {
        LogError("count != TEXT_COUNT: %d %d", count, TEXT_COUNT);
    }

    for (int i = 0; i < TEXT_COUNT; i++)
    {
        _english_texts[i] = strings + *offsets++;
    }

    delete[] buf;
}

void LanguageString::_InitTrans()
{
    _trans.clear();

    char *buf;
    File::ReadCompressedFile(TRANSLATION__ZBIN_PATH, (void **)&buf);

    char *en_strings;
    uint32_t *en_offsets;
    uint32_t en_count;
    _GetPoints((uint32_t *)buf, 0, &en_strings, &en_offsets, &en_count);
    if (en_count != TRANSLATION_COUNT)
    {
        LogError("en_count != TRANSLATION_COUNT: %d %d", en_count, TRANSLATION_COUNT);
    }

    char *strings;
    uint32_t *offsets;
    uint32_t count;
    _GetPoints((uint32_t *)buf, gConfig->language, &strings, &offsets, &count);
    if (count != TRANSLATION_COUNT)
    {
        LogError("count != TRANSLATION_COUNT: %d %d", count, TRANSLATION_COUNT);
    }

    for (int i = 0; i < TRANSLATION_COUNT; i++)
    {
        char *en = en_strings + *en_offsets++;
        char *s = strings + *offsets++;
        if (*s)
        {
            _trans[en] = s;
        }
    }

    delete[] buf;
}