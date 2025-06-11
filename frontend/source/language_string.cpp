#include "language_string.h"
#include "global.h"

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

    auto iter = gTrans.find(_string);
    if (iter == gTrans.end() || *(iter->second[gConfig->language - 1]) == '\x00')
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