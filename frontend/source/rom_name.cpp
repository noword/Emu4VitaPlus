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

    if (_name_buf)
    {
        delete[] _name_buf;
        _name_buf = nullptr;
    }
}

bool RomNameMap::_Load(const char *path)
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
        std::array<char *, LANGUAGE_COUNT + 1> offsets;
        for (size_t j = 0; j < LANGUAGE_COUNT + 1; j++)
        {
            offsets[j] = (char *)*p++;
        }
        _map[key] = offsets;
    }

    size = *p++;
    _name_buf = new char[size];
    memcpy(_name_buf, p, size);

    delete[] buf;

    for (auto &iter : _map)
    {
        auto &offsets = iter.second;
        for (size_t j = 0; j < LANGUAGE_COUNT + 1; j++)
        {
            offsets[j] = _name_buf + (uint32_t)offsets[j];
        }
    }

    LogDebug("  Load %d names from %s", _map.size(), path);
    LogDebug("  name buf size: 0x%x", size);

    return true;
}

bool RomNameMap::GetName(uint32_t crc, const char **name, int lang) const
{
    return _GetName(crc, name, lang + 1);
}

bool RomNameMap::GetRom(uint32_t crc, const char **name) const
{
    return _GetName(crc, name, 0);
}

bool RomNameMap::_GetName(uint32_t crc, const char **name, int index) const
{
    LogFunctionName;
    LogDebug("  crc32: %08x index: %d", crc, index);
    bool result = false;

    if (_name_buf != nullptr)
    {
        const auto &iter = _map.find(crc);
        if (iter != _map.end())
        {
            *name = iter->second[index];
            result = (**name != '\x00');
        }
    }

    if ((!result) && index != 1) // failed to get rom name or local name
        result = _GetName(crc, name, 1);

    if (result)
        LogDebug("rom name: %s", *name);

    return result;
}

void RomNameMap::Load()
{
    _map.clear();

    _ReleaseNameBuf();

    _Load("app0:assets/names.zdb") || _Load((std::string(CONSOLE_DIR) + "/names.zdb").c_str());
}