#pragma once
#include "tab_selectable.h"
#include "directory.h"
#include "dialog.h"
#include "rom_name.h"

enum
{
    CMD_PASTE = -1,
    CMD_COPY = 0,
    CMD_CUT,
    CMD_DELETE,
    CMD_RENAME,
    CMD_DOWNLOAD_THUMBNAILS
};

struct Clipboard
{
    std::string path;
    bool cut;
    bool Empty() { return path.empty(); };
    void Reset() { path.clear(); };
};

class TabBrowser : public TabSeletable
{
public:
    TabBrowser();
    virtual ~TabBrowser();
    virtual void SetInputHooks(Input *input) override;
    virtual void UnsetInputHooks(Input *input) override;
    virtual void Show(bool selected) override;
    virtual void ChangeLanguage(uint32_t language) override;
    bool Visable() override { return _visable; };

private:
    virtual void _Show() override;
    size_t _GetItemCount() override { return _directory->GetSize(); };
    bool _ItemVisable(size_t index) override { return index < _directory->GetSize(); };

    void _OnActive(Input *input) override;
    void _OnKeyCross(Input *input);
    void _OnKeyTriangle(Input *input);
    void _OnKeySquare(Input *input);
    void _OnKeyStart(Input *input);
    void _OnKeySelect(Input *input);
    void _OnDownloadThumbnails(Input *input);
    void _OnCancelDownloadThumbnails(Input *input);

    static int _DownloadThumbnailsThread(uint32_t args, void *argp);

    void _OnDialog(Input *input, int index);
    void _OnConfirmDialog(Input *input, int index);
    void _PasteFile(bool overwrite);

    void _UpdateStatus();
    void _UpdateInfo();
    void _Update() override;

    void _Search(const char *s);
    const std::string _GetCurrentFullPath(bool *is_dir = nullptr);
    void _SaveIndexHistory();
    void _SetIndexFromHistory();

    void _OnItemUpdated(DirItem *item);

    bool _TextInputCallback(const char *text);

    Directory *_directory;
    vita2d_texture *_texture;
    float _texture_width;
    float _texture_height;
    float _texture_max_width;
    float _texture_max_height;

    Input *_input;
    const char *_name;
    std::string _info;
    TextMovingStatus _name_moving_status;

    Clipboard _clipboard;
    Dialog *_dialog;
    Dialog *_confirm_dialog;
    int _cmd;
    bool _updating_thumbnails;

    std::map<std::string, size_t> _dir_history;
};