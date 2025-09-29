#pragma once
#include <unordered_map>
#include <array>
#include <stdint.h>
#include <string>
#include "language_define.h"

class RomNameMap
{
public:
    RomNameMap();
    virtual ~RomNameMap();

    void Load();

    bool GetName(uint32_t crc, const char **name, int lang) const;
    bool GetRom(uint32_t crc, const char **name) const;
    bool Valid() { return _map.size() > 0; };

private:
    bool _Load(const char *path);
    void _ReleaseNameBuf();
    bool _GetName(uint32_t crc, const char **name, int index) const;

    std::unordered_map<uint32_t, std::array<char *, LANGUAGE_COUNT + 1>> _map;
    char *_name_buf;
};