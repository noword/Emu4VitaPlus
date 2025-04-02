#pragma once
#include "tab_selectable.h"
#include "directory.h"
#include "dialog.h"
#include "rom_name.h"

int32_t GetNameThread(uint32_t args, void *argc);

enum
{
    CMD_PASTE = -1,
    CMD_COPY = 0,
    CMD_CUT,
    CMD_DELETE,
    CMD_RENAME,
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
    void Show(bool selected);
    virtual void ChangeLanguage(uint32_t language) override;
    bool Visable() override { return _visable; };

    friend int32_t GetNameThread(uint32_t args, void *argc);

private:
    size_t _GetItemCount() override { return _directory->GetSize(); };
    bool _ItemVisable(size_t index) override { return index < _directory->GetSize(); };

    void _OnActive(Input *input) override;
    void _OnKeyCross(Input *input);
    void _OnKeyTriangle(Input *input);
    void _OnKeySquare(Input *input);
    void _OnKeyStart(Input *input);
    void _OnKeySelect(Input *input);
    void _OnPsActive(Input *input);

    void _OnDialog(Input *input, int index);
    void _OnConfirmDialog(Input *input, int index);
    void _PasteFile(bool overwrite);

    void _UpdateTexture();
    void _UpdateStatus();
    void _UpdateName();
    void _Update() override;

    void _Search(const char *s);
    const std::string _GetCurrentFullPath(bool *is_dir = nullptr);
    void _SaveIndexHistory();
    void _SetIndexFromHistory();

    Directory *_directory;
    vita2d_texture *_texture;
    float _texture_width;
    float _texture_height;
    float _texture_max_width;
    float _texture_max_height;

    InputTextDialog *_text_dialog;
    Input *_input;
    RomNameMap _name_map;
    const char *_name;
    TextMovingStatus _name_moving_status;

    Clipboard _clipboard;
    Dialog *_dialog;
    Dialog *_confirm_dialog;
    int _cmd;

    std::map<std::string, size_t> _dir_history;
};