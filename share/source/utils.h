#pragma once
#include <string>
#include <functional>
#include <psp2/json.h>
#include "language_define.h"

#define ALIGN_UP(x, a) ((x) + ((a) - 1)) & ~((a) - 1)
#define ALIGN_UP_10H(x) ALIGN_UP(x, 0x10)

#define LOOP_PLUS_ONE(VALUE, TOTAL) (VALUE = (((VALUE + 1) >= (TOTAL)) ? 0 : VALUE + 1))
#define LOOP_MINUS_ONE(VALUE, TOTAL) (VALUE = ((VALUE == 0) ? (TOTAL) - 1 : VALUE - 1))

namespace Utils
{
    class JsonAllocator : public sce::Json::MemAllocator
    {
    public:
        JsonAllocator() {};
        virtual ~JsonAllocator() {};

        virtual void *allocateMemory(size_t size, void *user_data) override
        {
            return new uint8_t[size];
        }

        virtual void freeMemory(void *ptr, void *user_data) override
        {
            delete[] (uint8_t *)ptr;
        }
    };

    using CheckVersionCallback = std::function<void(bool)>;

    void Lower(std::string *s);
    void StripQuotes(std::string *s);
    void TrimString(std::string *s);
    uint32_t LowerCaseCrc32(const std::string s);

    int Utf16ToUtf8(const uint16_t *utf16, char *utf8, size_t size);
    int Utf8ToUtf16(const char *utf8, uint16_t *utf16, size_t size);

    LANGUAGE GetDefaultLanguage();

    void CheckVersion(CheckVersionCallback callback);
};