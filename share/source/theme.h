#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <vector>
#include <string>

#define DEFAULT_THEMES_JSON "app0:assets/themes.json"
#define DEFAULT_THEME_NAME "Dark"

extern ImU32 IM_COL32_GREEN;
extern ImU32 IM_COL32_RED;
extern ImU32 IM_COL32_YELLOW;

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
    size_t GetIndexByName(const char *name) const;
    void Apply(size_t index) const;
    void Apply(const char *name) const;
    const std::vector<Theme> &Get() const { return _themes; };

private:
    std::vector<Theme> _themes;
};