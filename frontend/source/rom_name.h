#pragma once
#include <unordered_map>
#include <array>
#include <stdint.h>
#include <string>
#include "language_define.h"

enum NAME_LANG
{
    NAME_ENGLISH = 0,
    NAME_LOCAL
};

class RomNameMap
{
public:
    RomNameMap();
    virtual ~RomNameMap();

    void Load();

    bool GetName(uint32_t crc, const char **name, NAME_LANG lang) const;
    bool Valid() { return _map.size() > 0; };

private:
    bool _Load(const char *path, NAME_LANG lang);
    bool _Load(const std::string &path, NAME_LANG lang);
    void _ReleaseNameBuf();

    std::unordered_map<uint32_t, std::array<uint32_t, 2>> _map;
    char *_name_buf[2];
};