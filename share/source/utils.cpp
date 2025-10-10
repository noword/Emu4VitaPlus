#include <imgui_vita2d/imgui_vita.h>
#include <psp2/power.h>
#include <algorithm>
#include <stdio.h>
#include <zlib.h>
#include "my_imgui.h"
#include "utils.h"
#include "log.h"

namespace Utils
{
    void Lower(std::string *s)
    {
        std::transform(s->begin(), s->end(), s->begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
    }

    void StripQuotes(std::string *s)
    {
        if (!s->empty() && s->front() == '"')
        {
            s->erase(s->begin());
        }

        if (!s->empty() && s->back() == '"')
        {
            s->erase(s->end() - 1);
        }
    }

    void TrimString(std::string *s)
    {
        s->erase(s->begin(), std::find_if(s->begin(), s->end(), [](unsigned char ch)
                                          { return !std::isspace(ch); }));
        s->erase(std::find_if(s->rbegin(), s->rend(), [](unsigned char ch)
                              { return !std::isspace(ch); })
                     .base(),
                 s->end());
    }

    uint32_t LowerCaseCrc32(const std::string s)
    {
        std::string _s{s};
        Lower(&_s);
        return crc32(0, (uint8_t *)_s.c_str(), _s.size());
    }

    int Utf16leToUtf8(uint16_t utf16_char, char *utf8_char)
    {
        if (utf16_char < 0x80)
        {
            utf8_char[0] = (char)utf16_char;
            return 1;
        }
        else if (utf16_char < 0x800)
        {
            utf8_char[0] = 0xC0 | (utf16_char >> 6);
            utf8_char[1] = 0x80 | (utf16_char & 0x3F);
            return 2;
        }
        else
        {
            utf8_char[0] = 0xE0 | (utf16_char >> 12);
            utf8_char[1] = 0x80 | ((utf16_char >> 6) & 0x3F);
            utf8_char[2] = 0x80 | (utf16_char & 0x3F);
            return 3;
        }
    }

    std::string Utf16leToUtf8(uint16_t *utf16_str)
    {
        std::string utf8_str;
        char utf8_char[4]; // Buffer to hold UTF-8 characters (max 4 bytes)

        while (*utf16_str)
        {
            int bytes = Utf16leToUtf8(*utf16_str, utf8_char);
            utf8_str.append(utf8_char, bytes);
            utf16_str++;
        }

        return utf8_str;
    }

    int Utf16ToUtf8(const uint16_t *utf16, char *utf8, size_t size)
    {
        int count = 0;
        while (*utf16)
        {
            uint32_t code;
            uint16_t ch = *utf16++;
            if (ch < 0xd800 || ch >= 0xe000)
            {
                code = ch;
            }
            else // surrogate pair
            {
                uint16_t ch2 = *utf16++;
                if (ch < 0xdc00 || ch > 0xe000 || ch2 < 0xd800 || ch2 > 0xdc00)
                {
                    return count;
                }
                code = 0x10000 + ((ch & 0x03FF) << 10) + (ch2 & 0x03FF);
            }

            if (code < 0x80)
            {
                if (size < 1)
                    return count;
                utf8[count++] = (char)code;
                size--;
            }
            else if (code < 0x800)
            {
                if (size < 2)
                    return count;
                utf8[count++] = (char)(0xc0 | (code >> 6));
                utf8[count++] = (char)(0x80 | (code & 0x3f));
                size -= 2;
            }
            else if (code < 0x10000)
            {
                if (size < 3)
                    return count;
                utf8[count++] = (char)(0xe0 | (code >> 12));
                utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
                utf8[count++] = (char)(0x80 | (code & 0x3f));
                size -= 3;
            }
            else
            {
                if (size < 4)
                    return count;
                utf8[count++] = (char)(0xf0 | (code >> 18));
                utf8[count++] = (char)(0x80 | ((code >> 12) & 0x3f));
                utf8[count++] = (char)(0x80 | ((code >> 6) & 0x3f));
                utf8[count++] = (char)(0x80 | (code & 0x3f));
                size -= 4;
            }
        }
        utf8[count] = '\x00';
        return count;
    }

    int Utf8ToUtf16(const char *utf8, uint16_t *utf16, size_t size)
    {
        int count = 0;
        while (*utf8)
        {
            uint8_t ch = (uint8_t)*utf8++;
            uint32_t code;
            uint32_t extra;

            if (ch < 0x80)
            {
                code = ch;
                extra = 0;
            }
            else if ((ch & 0xe0) == 0xc0)
            {
                code = ch & 31;
                extra = 1;
            }
            else if ((ch & 0xf0) == 0xe0)
            {
                code = ch & 15;
                extra = 2;
            }
            else
            {
                // TODO: this assumes there won't be invalid utf8 codepoints
                code = ch & 7;
                extra = 3;
            }

            for (uint32_t i = 0; i < extra; i++)
            {
                uint8_t next = (uint8_t)*utf8++;
                if (next == 0 || (next & 0xc0) != 0x80)
                {
                    return count;
                }
                code = (code << 6) | (next & 0x3f);
            }

            if (code < 0xd800 || code >= 0xe000)
            {
                if (size < 1)
                    return count;
                utf16[count++] = (uint16_t)code;
                size--;
            }
            else // surrogate pair
            {
                if (size < 2)
                    return count;
                code -= 0x10000;
                utf16[count++] = 0xd800 | (code >> 10);
                utf16[count++] = 0xdc00 | (code & 0x3ff);
                size -= 2;
            }
        }
        utf16[count] = 0;
        return count;
    }

    LANGUAGE GetDefaultLanguage()
    {
        int sys_lang;
        LANGUAGE lang;
        sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &sys_lang);
        switch (sys_lang)
        {
        case SCE_SYSTEM_PARAM_LANG_JAPANESE:
            lang = LANGUAGE_JAPANESE;
            break;

        case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
        case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
            lang = LANGUAGE_CHINESE;
            break;

        case SCE_SYSTEM_PARAM_LANG_FRENCH:
            lang = LANGUAGE_FRENCH;
            break;

        case SCE_SYSTEM_PARAM_LANG_SPANISH:
            lang = LANGUAGE_SPANISH;
            break;

        case SCE_SYSTEM_PARAM_LANG_ITALIAN:
            lang = LANGUAGE_ITALIAN;
            break;

        case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
            lang = LANGUAGE_RUSSIAN;
            break;

        case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:
        case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:
        default:
            lang = LANGUAGE_ENGLISH;
        }
        return lang;
    }

};
