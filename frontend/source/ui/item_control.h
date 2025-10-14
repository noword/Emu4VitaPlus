#pragma once
#include <stdint.h>
#include <algorithm>
#include "item_selectable.h"
#include "config.h"
#include "defines.h"
#include "global.h"
#include "input_descriptor.h"
#include "icons.h"

using namespace Emu4VitaPlus;

class ItemControl : public ItemSelectable
{
public:
    ItemControl(ControlMapConfig *control_map)
        : ItemSelectable(Emu4VitaPlus::Config::ControlTextMap.at(control_map->psv), std::string(BUTTON_TRIANGLE) + TEXT(LANG_TURBO), std::bind(&Emulator::SetupKeysWithSaveConfig, gEmulator)),
          _control_map(control_map) {};
    virtual ~ItemControl() {};

    virtual void Show(bool selected) override
    {
        ItemSelectable::Show(selected);
        ImGui::SameLine();
        _control_map->turbo ? ImGui::TextUnformatted(TEXT(LANG_TURBO)) : ImGui::TextDisabled(TEXT(LANG_TURBO));
    };

    virtual void OnOption(Input *input) override
    {
        _control_map->turbo = !_control_map->turbo;
        gEmulator->SetupKeysWithSaveConfig();
    };

private:
    virtual size_t _GetTotalCount() override
    {
        return RETRO_KEYS.size();
    };

    virtual const char *_GetOptionString(size_t index) override
    {
        static std::string s;
        s = gConfig->input_descriptors.Get(RETRO_KEYS[index]);
        if (std::find(_control_map->retros.begin(), _control_map->retros.end(), RETRO_KEYS[index]) != _control_map->retros.end())
        {
            s += ICON_STAR_SPACE;
        }

        return s.c_str();
    };

    virtual const char *_GetPreviewText() override
    {
        _preview.clear();
        for (const auto &r : _control_map->retros)
        {
            if (_preview.size() > 0)
            {
                _preview += " + ";
            }
            _preview += gConfig->input_descriptors.Get(r);
        }
        return _preview.c_str();
    };

    virtual void _OnClick(Input *input) override
    {
        uint8_t r = RETRO_KEYS[_index];
        std::vector<uint8_t> *retros = &_control_map->retros;
        auto iter = std::find(retros->begin(), retros->end(), r);
        if (iter == retros->end())
        {
            retros->push_back(r);
        }
        else
        {
            retros->erase(iter);
        }

        ItemBase::OnActive(input);
    };

    virtual bool _IsHighlight(size_t index) override
    {
        return index == _index;
        //|| std::find(_control_map->retros.begin(), _control_map->retros.end(), RETRO_KEYS[index]) != _control_map->retros.end();
    }

    ControlMapConfig *_control_map;
    std::string _preview;
};