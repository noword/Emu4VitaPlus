#pragma once
#include <string>
#include "defines.h"

class Config
{
public:
    Config();
    virtual ~Config();

    bool Load(const char *path = APP_CONFIG_PATH);
    bool Save(const char *path = APP_CONFIG_PATH);

    std::string last_core;
    uint32_t language;

private:
};

extern Config *gConfig;