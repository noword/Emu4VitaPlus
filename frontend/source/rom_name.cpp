#include "rom_name.h"
#include "file.h"
#include "config.h"
#include "language_string.h"
#include "log.h"
#include "global.h"

RomNameMap::RomNameMap()
    : _name_buf{nullptr}
{
}

RomNameMap::~RomNameMap()
{
    _ReleaseNameBuf();
}

void RomNameMap::_ReleaseNameBuf()
{
    for (int i = 0; i < 2; i++)
    {
        if (_name_buf[i] != nullptr)
        {
            delete[] _name_buf[i];
            _name_buf[i] = nullptr;
        }
    }
}

bool RomNameMap::_Load(const char *path, NAME_LANG lang)
{
    LogFunctionName;

    char *buf;
    if (File::ReadCompressedFile(path, (void **)&buf) == 0)
    {
        LogError("failed to load %s", path);
        return false;
    }

    uint32_t *p = (uint32_t *)buf;

    uint32_t size = *p++;

    if (_map.size() < size)
        _map.reserve(size);

    for (size_t i = 0; i < size; i++)
    {
        uint32_t key = *p++;
        uint32_t offset = *p++;
        auto iter = _map.find(key);
        if (iter == _map.end())
        {
            std::array<uint32_t, 2> value{0, 0};
            value[lang] = offset;
            _map.emplace(key, value);
        }
        else
        {
            iter->second[lang] = offset;
        }
    }

    size = *p++;
    _name_buf[lang] = new char[size];
    memcpy(_name_buf[lang], p, size);

    delete[] buf;

    LogDebug("  Load %d names from %s", _map.size(), path);
    LogDebug("  name buf size: 0x%x", size);

    return true;
}

bool RomNameMap::_Load(const std::string &path, NAME_LANG lang)
{
    return _Load(path.c_str(), lang);
}

bool RomNameMap::GetName(uint32_t crc, const char **name, NAME_LANG lang) const
{
    LogFunctionName;
    LogDebug("  crc32: %08x", crc);
    const char *name_buf = _name_buf[lang];
    if (name_buf == nullptr)
        return false;

    const auto &iter = _map.find(crc);
    if (iter == _map.end() || iter->second[lang] == 0)
    {
        return false;
    }

    *name = name_buf + iter->second[lang];

    if (**name == '\x00' && lang == NAME_LOCAL && _name_buf[NAME_ENGLISH])
    {
        *name = _name_buf[NAME_ENGLISH] + iter->second[NAME_ENGLISH];
    }
    LogDebug("rom name: %s", *name);
    return true;
}

void RomNameMap::Load()
{
    _map.clear();

    _ReleaseNameBuf();

    _Load("app0:assets/names.en.zdb", NAME_ENGLISH) || _Load((std::string(CONSOLE_DIR) + "/names.en.zdb").c_str(), NAME_ENGLISH);
    _Load(std::string("app0:assets/names.") + TEXT(CODE) + ".zdb", NAME_LOCAL) | _Load(std::string(CONSOLE_DIR) + "/names." + TEXT(CODE) + ".zdb", NAME_LOCAL);
}