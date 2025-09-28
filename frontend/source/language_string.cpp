#include "language_string.h"
#include "global.h"

std::unordered_map<std::string, TRANS> LanguageString::_trans;

const char *TEXT(size_t index)
{
    return index < TEXT_COUNT ? gTexts[gConfig->language][index] : "Unknown";
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
    if (iter == _trans.end() || *(iter->second[gConfig->language - 1]) == '\x00')
    {
        return _string.c_str();
    }
    else
    {
        return iter->second[gConfig->language - 1];
    }
}

const char *const LanguageString::GetOriginal() const
{
    if (_text_id != INVALID_TEXT_ENUM)
    {
        return _text_id < TEXT_COUNT ? gTexts[0][_text_id] : "Unknown";
    }

    return _string.c_str();
}

void LanguageString::InitTrans()
{
    if (_trans.size() > 0)
    {
        _trans.clear();
    }

    _trans.reserve(TRANSLATION_COUNT);
    for (int i = 0; i < TRANSLATION_COUNT; i++)
    {
        TRANS t;
        for (int j = 0; j < LANGUAGE_COUNT - 1; j++)
        {
            t[j] = gTrans[i][j + 1];
        }
        _trans.emplace(std::make_pair(gTrans[i][0], t));
    }
}