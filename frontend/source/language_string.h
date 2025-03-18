#pragma once
#include <string>
#include "language_define.h"
#include "language_frontend.h"

#define INVALID_TEXT_ENUM TEXT_ENUM(-1)

extern const char *TEXT(size_t index);

class LanguageString
{
public:
    LanguageString(TEXT_ENUM text_id) : _text_id(text_id) {};
    LanguageString(const char *str) : _string(str), _text_id(INVALID_TEXT_ENUM) {};
    LanguageString(const std::string str) : _string(str), _text_id(INVALID_TEXT_ENUM) {};
    LanguageString(size_t id) : _text_id(TEXT_ENUM(id)) {};
    LanguageString(const LanguageString &ls) : _text_id(ls._text_id), _string(ls._string) {};

    const char *const Get() const;
    const char *const GetOriginal() const;

private:
    TEXT_ENUM _text_id;
    std::string _string;
};