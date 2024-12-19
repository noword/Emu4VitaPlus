#include <psp2/io/dirent.h>
#include <algorithm>
#include <cctype>
#include <string.h>
#include "directory.h"
#include "utils.h"
#include "log.h"
#include "file.h"
#include "archive_reader_factory.h"

static const size_t INPUT_7Z_BUF_SIZE = 1 << 18;

static void SortDirItemsByNameIgnoreCase(std::vector<DirItem> &items)
{
    std::sort(items.begin(), items.end(), [](const DirItem &a, const DirItem &b)
              { return std::lexicographical_compare(
                    a.name.begin(), a.name.end(), b.name.begin(), b.name.end(),
                    [](unsigned char ch1, unsigned char ch2)
                    { return std::tolower(ch1) < std::tolower(ch2); }); });
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

bool Directory::_LeagleTest(const char *name, DirItem *item)
{
    std::string ext = File::GetExt(name);

    if (_ext_filters.find(ext) != _ext_filters.end())
    {
        return true;
    }

    if (!item)
    {
        return false;
    }

    if (File::GetSize(name) > 50000000)
    {
        return false;
    }

    ArchiveReader *reader = gArchiveReaderFactory->Get(name);
    if (reader == nullptr || !reader->Open((_current_path + "/" + name).c_str()))
    {
        return false;
    }

    bool result = false;
    do
    {
        const char *entry_name = reader->GetCurrentName();
        result = _LeagleTest(entry_name);
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

    SceIoDirent dir;
    while (sceIoDread(dfd, &dir) > 0)
    {
        if (*dir.d_name == '.')
        {
            continue;
        }
        else if (SCE_S_ISDIR(dir.d_stat.st_mode))
        {
            _items.push_back({dir.d_name, true});
        }
        else
        {
            DirItem item{dir.d_name, false};
            if (_LeagleTest(dir.d_name, &item))
            {
                files.push_back(item);
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
            _items.push_back({device, true});
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
        if ((!item.is_dir) && (item.name.find(s) != std::string::npos || item.entry_name.find(s) != std::string::npos))
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

int Directory::GetIndex(const char *name)
{
    int count = 0;
    for (const auto &item : _items)
    {
        if (item.name == name)
        {
            return count;
        }
        count++;
    }

    return -1;
}