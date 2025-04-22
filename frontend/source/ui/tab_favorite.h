#pragma once
#include <vita2d.h>
#include "tab_selectable.h"
#include "favorite.h"
#include "dialog.h"

class TabFavorite : virtual public TabSeletable
{
public:
    TabFavorite();
    virtual ~TabFavorite();
    void Show(bool selected);
    void SetInputHooks(Input *input);
    void UnsetInputHooks(Input *input);
    virtual void ChangeLanguage(uint32_t language) override;

private:
    size_t _GetItemCount() override { return gFavorites->size(); };
    bool _ItemVisable(size_t index) override { return true; };
    void _OnActive(Input *input) override;
    void _OnKeyCross(Input *input);
    void _OnDialog(Input *input, int index);
    void _UpdateStatus();
    void _UpdateTexture();
    void _UpdateInfo();
    void _Update() override;

    Dialog *_dialog;
    vita2d_texture *_texture;
    float _texture_width;
    float _texture_height;
    float _texture_max_width;
    float _texture_max_height;
    TextMovingStatus _name_moving_status;
    std::string _info;
};