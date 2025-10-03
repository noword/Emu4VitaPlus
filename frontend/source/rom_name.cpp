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
    uint32_t buf_size = *p++;
    _name_buf = new char[buf_size];
    memcpy(_name_buf, p, buf_size);
    LogDebug("_name_buf %08x", _name_buf);

    p += buf_size / sizeof(uint32_t);
    uint32_t size = *p++;

    if (_map.size() < size)
        _map.reserve(size);

    for (size_t i = 0; i < size; i++)
    {
        uint32_t key = *p++;
        std::array<char *, LANGUAGE_COUNT + 1> offsets;
        for (size_t j = 0; j < LANGUAGE_COUNT + 1; j++)
        {
            offsets[j] = _name_buf + *p++;
        }
        _map[key] = offsets;
    }

    LogDebug("  Load %d names from %s", _map.size(), path);
    LogDebug("  name buf size: 0x%x", buf_size);

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
            if ((!result) && index != 1) // failed to get rom name or local name
            {
                *name = iter->second[1];
                result = (**name != '\x00');
            }
        }
    }

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