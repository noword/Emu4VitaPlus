#include <psp2/io/dirent.h>
#include <algorithm>
#include <cctype>
#include <string.h>
#include <zlib.h>
#include "directory.h"
#include "global.h"
#include "utils.h"
#include "log.h"
#include "file.h"
#include "archive_reader_factory.h"
#include "thread_base.h"

#define THRESHOLD_FILE_COUNT 200

static void SortDirItemsByNameIgnoreCase(std::vector<DirItem> &items)
{
    std::sort(items.begin(), items.end(), [](const DirItem &a, const DirItem &b)
              { return std::lexicographical_compare(
                    a.path.begin(), a.path.end(), b.path.begin(), b.path.end(),
                    [](unsigned char ch1, unsigned char ch2)
                    { return std::tolower(ch1) < std::tolower(ch2); }); });
}

struct UpdateDetialsArgument
{
    DirItem *item;
    DirItemUpdateCallbackFunc callback;
};

uint32_t GetRomCrc32(const char *full_path)
{
    uint32_t crc = 0;

    const ArcadeManager *arc_manager = gEmulator->GetArcadeManager();
    if (arc_manager)
    {
        std::string rom_name = File::GetName(arc_manager->GetRomName(full_path));
        crc = crc32(0, (Bytef *)rom_name.c_str(), rom_name.size());
    }
    else if (File::GetSize(full_path) < 50 * 1024 * 1024)
    {
        crc = File::GetCrc32(full_path);
    }

    return crc;
}

int32_t UpdateDetialsThread(uint32_t args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(UpdateDetialsArgument, argument, argp);

    DirItem *item = argument->item;
    const std::string full_path = item->GetFullPath();

    if (item->crc32 == 0)
    {
        item->crc32 = GetRomCrc32(full_path.c_str());
        LogDebug("crc32: %08x", item->crc32);
    }

    if (item->english_name.empty() && item->display_name.empty())
    {
        if (gPlaylists->IsValid())
        {
            const char *label = gPlaylists->GetLabel(full_path.c_str());
            if (label)
            {
                item->display_name = label;
            }
        }

        if (gRomNameMap->Valid() && item->crc32)
        {
            const char *local_name;
            const char *english_name;
            gRomNameMap->GetName(item->crc32, &local_name, &english_name);
            if (local_name && *local_name && item->display_name.empty())
            {
                item->display_name = local_name;
            }

            if (english_name && *english_name)
            {
                item->english_name = english_name;
                if (item->display_name.empty())
                {
                    item->display_name = english_name;
                }
            }
        }
    }

    LogDebug("  english_name: %s  display_name: %s", item->english_name.c_str(), item->display_name.c_str());

    DirItemUpdateCallbackFunc callback = argument->callback;
    delete argument;

    if (callback)
    {
        callback(item);
    }

    return sceKernelExitDeleteThread(0);
}

void DirItem::UpdateDetails(DirItemUpdateCallbackFunc callback)
{
    LogFunctionName;
    if (!is_dir)
    {
        UpdateDetialsArgument *argument = new UpdateDetialsArgument{this, callback};
        if (callback)
            StartThread(UpdateDetialsThread, sizeof(UpdateDetialsArgument), &argument);
        else
            UpdateDetialsThread(sizeof(UpdateDetialsArgument), &argument);
    }
}

Directory::Directory(const char *path, const char *ext_filters, char split)
{
    LogFunctionName;

    if (ext_filters)
    {
        SetExtensionFilter(ext_filters, split);
    }

    if (path)
    {
        SetCurrentPath(path);
    }
}

Directory::~Directory()
{
    LogFunctionName;
}

void Directory::SetExtensionFilter(const char *exts, char split)
{
    LogFunctionName;
    LogDebug(exts);

    char *exts_string = new char[strlen(exts) + 1];
    strcpy(exts_string, exts);
    char *p = exts_string;
    char *end;
    do
    {
        end = strchr(p, split);
        if (end)
        {
            *end = '\x00';
        }
        _ext_filters.insert(p);

        if (end)
        {
            end++;
            p = end;
        }
        else
        {
            break;
        }
    } while (true);

    delete[] exts_string;
}

bool Directory::_SuffixTest(const char *name)
{
    return (_ext_filters.find(File::GetExt(name)) != _ext_filters.end());
}

bool Directory::LegalTest(const char *path, DirItem *item)
{
    // LogDebug("  _LeagleTest: %s", name);
    if (_SuffixTest(path))
    {
        return true;
    }

    if (!item)
    {
        return false;
    }

    // if (File::GetSize(name) > 50000000)
    // {
    //     return false;
    // }

    ArchiveReader *reader = gArchiveReaderFactory->Get(path);
    if (reader == nullptr || !reader->Open((_current_path + "/" + path).c_str()))
    {
        return false;
    }

    bool result = false;
    do
    {
        const char *entry_name = reader->GetCurrentName();
        result = _SuffixTest(entry_name);
        if (result)
        {
            item->entry_name = entry_name;
            item->crc32 = reader->GetCurrentCrc32();
            break;
        }
    } while (reader->Next());

    return result;
}

bool Directory::SetCurrentPath(const std::string &path)
{
    LogFunctionName;
    LogDebug("  path: %s", path.c_str());

    _search_results.clear();

    if (path.size() == 0)
    {
        return _ToRoot();
    }

    _items.clear();

    SceUID dfd = sceIoDopen(path.c_str());
    if (dfd < 0)
    {
        return false;
    }

    _current_path = path;
    std::vector<DirItem> files;
    size_t file_count = File::GetFileCount(path.c_str());
    files.reserve(file_count);
    _tested = file_count <= THRESHOLD_FILE_COUNT;

    SceIoDirent dir;
    while (sceIoDread(dfd, &dir) > 0)
    {
        if (*dir.d_name == '.')
        {
            continue;
        }
        else if (SCE_S_ISDIR(dir.d_stat.st_mode))
        {
            _items.push_back({&_current_path, dir.d_name, true});
        }
        else if (_tested)
        {
            DirItem item{&_current_path, dir.d_name, false};
            if (LegalTest(dir.d_name, &item))
            {
                files.emplace_back(item);
            }
        }
        else
        {
            DirItem item{&_current_path, dir.d_name, false};
            if (_SuffixTest(dir.d_name))
            {
                files.emplace_back(item);
            }
            else
            {
                std::string ext = File::GetExt(dir.d_name);
                if (ext == "zip" || ext == "7z")
                {
                    files.emplace_back(item);
                }
            }
        }
    }
    sceIoDclose(dfd);

    SortDirItemsByNameIgnoreCase(_items);
    SortDirItemsByNameIgnoreCase(files);

    _items.insert(_items.end(), files.begin(), files.end());
    LogDebug("items %d", _items.size());
    return true;
}

size_t Directory::GetSize()
{
    return _items.size();
}

bool Directory::_ToRoot()
{
    _items.clear();
    _current_path = "";

    for (const auto device : {
             "imc0:",
             "uma0:",
             "ur0:",
             "ux0:",
             "xmc0:",
         })
    {
        if (File::Exist(device))
        {
            _items.push_back({&_current_path, device, true});
        }
    }

    return _items.size() > 0;
}

size_t Directory::Search(const char *s)
{
    _search_results.clear();
    if (s == nullptr || *s == '\x00')
    {
        _search_str = "";
        return 0;
    }

    _search_str = s;
    size_t count = 0;
    for (const auto &item : _items)
    {
        if ((!item.is_dir) && (item.path.find(s) != std::string::npos || item.entry_name.find(s) != std::string::npos))
        {
            _search_results.insert(count);
        }
        count++;
    }

    return _search_results.size();
}

bool Directory::BeFound(size_t index)
{
    return _search_results.find(index) != _search_results.end();
}

int Directory::GetIndex(const char *path)
{
    int count = 0;
    for (const auto &item : _items)
    {
        if (item.path == path)
        {
            return count;
        }
        count++;
    }

    return -1;
}