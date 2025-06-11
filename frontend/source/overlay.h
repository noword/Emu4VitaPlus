#pragma once
#include <string>
#include <vector>
#include <vita2d.h>
#include "language_string.h"

class Overlay
{
public:
    Overlay();
    virtual ~Overlay();
    vita2d_texture *Get();

    std::string name;
    std::string image_name;
    int viewport_rotate;
    int viewport_width;
    int viewport_height;
    int viewport_x;
    int viewport_y;

private:
    vita2d_texture *_texture;
};

class Overlays : public std::vector<Overlay>
{
public:
    Overlays();
    virtual ~Overlays();
    bool Load(const char *path);
    std::vector<LanguageString> GetConfigs();
};
