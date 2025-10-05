#pragma once
#include <unordered_map>
#include <array>
#include <string>
#include "language_define.h"
#include "language_frontend.h"

#define INVALID_TEXT_ENUM TEXT_ENUM(-1)

typedef std::array<const char *, LANGUAGE_COUNT - 1> TRANS;

extern const char *TEXT(size_t index);

class LanguageString
{
public:
    static void Init();
    static const std::array<std::string, TEXT_COUNT> &Texts() { return _texts; };

    LanguageString(TEXT_ENUM text_id) : _text_id(text_id) {};
    LanguageString(const char *str) : _string(str), _text_id(INVALID_TEXT_ENUM) {};
    LanguageString(const std::string str) : _string(str), _text_id(INVALID_TEXT_ENUM) {};
    LanguageString(size_t id) : _text_id(TEXT_ENUM(id)) {};
    LanguageString(const LanguageString &ls) : _text_id(ls._text_id), _string(ls._string) {};

    virtual ~LanguageString() {};

    const char *const Get() const;
    const char *const GetOriginal() const;
    TEXT_ENUM GetId() const { return _text_id; };

private:
    static void _InitTexts();
    static void _InitTrans();

    TEXT_ENUM _text_id;
    std::string _string;

    static std::array<std::string, TEXT_COUNT> _english_texts;
    static std::array<std::string, TEXT_COUNT> _texts;
    static std::unordered_map<std::string, std::string> _trans;
};