#include "rom_name.h"
#include "file.h"
#include "config.h"
#include "language_string.h"
#include "log.h"
#include "global.h"

RomNameMap::RomNameMap(const char *path) : _name_buf(nullptr)
{
    if (path != nullptr)
    {
        Load(path);
    }
}

RomNameMap::~RomNameMap()
{
    if (_name_buf != nullptr)
        delete[] _name_buf;
}

bool RomNameMap::Load(const char *path)
{
    LogFunctionName;
    _map.clear();
    if (_name_buf != nullptr)
    {
        delete[] _name_buf;
        _name_buf = nullptr;
    }

    char *buf;
    if (File::ReadCompressedFile(path, (void **)&buf) == 0)
    {
        LogError("failed to load %s", path);
        return false;
    }

    uint32_t *p = (uint32_t *)buf;

    uint32_t size = *p++;
    _map.reserve(size);
    for (size_t i = 0; i < size; i++)
    {
        uint32_t key = *p++;
        uint32_t value = *p++;
        _map.emplace(key, value);
    }

    size = *p++;
    _name_buf = new char[size];
    memcpy(_name_buf, p, size);

    delete[] buf;

    LogDebug("  Load %d names from %s", _map.size(), path);
    LogDebug("  name buf size: 0x%x", size);

    return true;
}

bool RomNameMap::Load(const std::string &path)
{
    return Load(path.c_str());
}

bool RomNameMap::GetName(uint32_t crc, const char **name) const
{
    LogFunctionName;

    if (_name_buf == nullptr)
        return false;

    const auto &iter = _map.find(crc);
    if (iter == _map.end())
    {
        return false;
    }

    *name = _name_buf + iter->second;

    if (*name)
    {
        LogDebug("rom name: %s", *name);
    }

    return true;
}

void RomNameMap::Load()
{
    bool result = Load(std::string("app0:assets/names.") + TEXT(CODE) + ".zdb") || Load(std::string(CONSOLE_DIR) + "/names." + TEXT(CODE) + ".zdb");
    if (!result && gConfig->language != LANGUAGE_ENGLISH)
    {
        Load("app0:assets/names.en.zdb") || Load((std::string(CONSOLE_DIR) + "/names.en.zdb").c_str());
    }
}