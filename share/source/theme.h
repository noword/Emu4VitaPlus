#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <vector>
#include <string>

#define DEFAULT_THEMES_JSON "app0:assets/themes.json"

struct Theme
{
    std::string name;
    ImGuiStyle style;

    void Apply();
};

class Themes
{
public:
    Themes() {};
    Themes(const char *path) { Load(path); };
    virtual ~Themes() {};

    bool Load(const char *path);
    void Apply(size_t index);
    void Apply(const char *name);

private:
    std::vector<Theme> _themes;
};