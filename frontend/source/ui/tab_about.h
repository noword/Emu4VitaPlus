#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <vita2d.h>
#include "tab_base.h"

class TabAbout : public virtual TabBase
{
public:
    TabAbout();
    virtual ~TabAbout();
    virtual void SetInputHooks(Input *input) override;
    virtual void UnsetInputHooks(Input *input) override;

private:
    virtual void _Show() override;
    void _InitTexts();
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);

    size_t _index;
    std::vector<std::string> _texts;
    uint32_t _last_lang;
    vita2d_texture *_title_texture;
    float _title_index;
    int _input_count;
};