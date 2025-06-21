#pragma once
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <SimpleIni.h>
#include <libretro.h>
#include "language_string.h"
#include "defines.h"

struct ControllerType
{
    std::string desc;
    uint32_t device;
};

struct ControllerTypes : public std::vector<ControllerType>
{
    std::string value;

    const std::vector<LanguageString> GetValues() const;
    size_t GetValueIndex();
    void SetValueIndex(size_t index);
    void Default();
    void Apply(uint32_t port) const;
};

class DeviceOptions : public std::vector<ControllerTypes>
{
public:
    DeviceOptions();
    virtual ~DeviceOptions();

    bool Load(CSimpleIniA &ini);
    bool Save(CSimpleIniA &ini);

    void Load(retro_controller_info *info);

private:
};