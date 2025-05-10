#pragma once
#include <libretro.h>
#include <SimpleIni.h>
#include "language_string.h"
#include "global.h"

#define INPUT_DESC_COUNT 16

class InputDescriptor
{
public:
    InputDescriptor(const char *icon) : _icon(icon), _full(icon), _desc("") {};
    virtual ~InputDescriptor() {}

    const char *Get() { return _full.c_str(); };
    void Update() { _full = _icon + " " + _desc.Get(); };
    const char *GetDesc() { return _desc.GetOriginal(); };

    void SetDescription(const char *desc)
    {
        _desc = LanguageString(desc);
        Update();
    };

private:
    std::string _icon;
    LanguageString _desc;
    std::string _full;
};

class InputDescriptors
{
public:
    InputDescriptors();
    virtual ~InputDescriptors();
    void UpdateInputDescriptors(const retro_input_descriptor *descriptors);
    void Update();
    const char *Get(int index);
    bool Load(const char *path = CORE_INPUT_DESC_PATH);
    bool Load(CSimpleIniA &ini);
    bool Save(const char *path = CORE_INPUT_DESC_PATH);
    bool Save(CSimpleIniA &ini);

private:
    InputDescriptor _descriptors[INPUT_DESC_COUNT];
};
