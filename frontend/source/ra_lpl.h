#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <vita2d.h>

struct RomItem
{
    uint8_t db_name_index;
    std::string label;
};

using ItemMap = std::unordered_map<uint32_t, RomItem>;

class RetroArchPlaylists
{
public:
    RetroArchPlaylists();
    virtual ~RetroArchPlaylists();
    void LoadAll();
    void Load(const char *dir_path);
    const char *GetLabel(const char *path);
    vita2d_texture *GetPreviewImage(const char *path);
    bool IsValid() { return _items.size() > 0; };

private:
    uint32_t _GetLplCrc32(const char *path);
    bool _LoadLpl(const char *lpl_path, ItemMap &items);
    bool _LoadCache(uint32_t crc32, ItemMap &items);
    bool _SaveCache(uint32_t crc32, const ItemMap &items);
    uint8_t _GetDbIndex(const char *name);

    ItemMap _items;
    std::vector<std::string> _dbs;
};
