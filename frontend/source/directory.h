#pragma once
#include <vector>
#include <set>
#include <string>
#include <functional>
#include <psp2common/kernel/threadmgr.h>

struct DirItem;
typedef std::function<void(DirItem *)> DirItemUpdateCallbackFunc;

struct DirItem
{
    const std::string *parent_path;
    std::string path;
    bool is_dir;
    uint32_t crc32 = 0;
    bool legal = true;
    std::string entry_name = "";
    std::string display_name = "";
    std::string english_name = "";

    const std::string GetFullPath() { return (*parent_path) + '/' + path; }
    // update crc32, display_name and english_name
    void UpdateDetials(DirItemUpdateCallbackFunc callback = nullptr); // asynchronous
    void UpdateDetails();                                             // synchronous
};

class Directory
{
public:
    Directory(const char *path = nullptr,
              const char *ext_filters = NULL,
              char split = '|');
    virtual ~Directory();

    void SetExtensionFilter(const char *exts, char split);
    bool SetCurrentPath(const std::string &path);
    bool Refresh() { return SetCurrentPath(_current_path.c_str()); };
    const std::string &GetCurrentPath() const { return _current_path; };
    DirItem &GetItem(int index) { return _items[index]; };
    const std::string &GetItemPath(int index) const { return _items[index].path; };
    const bool IsDir(int index) const { return _items[index].is_dir; };
    size_t GetSize();
    size_t Search(const char *s);
    bool BeFound(size_t index);
    const std::string &GetSearchString() const { return _search_str; };
    const std::set<size_t> &GetSearchResults() const { return _search_results; };
    int GetIndex(const char *path);
    bool LegalTest(const char *path, DirItem *item = nullptr);
    bool IsTested() { return _tested; };

private:
    std::vector<DirItem> _items;
    std::set<std::string> _ext_filters;
    std::set<size_t> _search_results;
    std::string _current_path;
    std::string _search_str;
    bool _tested;

    inline bool _SuffixTest(const char *name);
    bool _ToRoot();
};