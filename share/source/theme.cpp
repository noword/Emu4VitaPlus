#include <algorithm>
#include <psp2/json.h>
#include "utils.h"
#include "theme.h"
#include "file.h"
#include "log.h"

ImU32 IM_COL32_GREEN;
ImU32 IM_COL32_RED;
ImU32 IM_COL32_YELLOW;

static ImGuiDir GetImGuiDir(const char *s)
{
    if (strcmp(s, "Left") == 0)
        return ImGuiDir_Left;
    else if (strcmp(s, "Right") == 0)
        return ImGuiDir_Right;
    else if (strcmp(s, "Up") == 0)
        return ImGuiDir_Up;
    else if (strcmp(s, "Down") == 0)
        return ImGuiDir_Down;
    else
        return ImGuiDir_None;
}

static Theme GetTheme(const sce::Json::Value &value)
{
#define GET_STR(A, THEME, VALUE) THEME.A = VALUE.getValue(#A).getString().c_str();
#define GET_FLOAT(A, THEME, VALUE) THEME.style.A = VALUE.getValue(#A).getReal();
#define GET_VEC2(A, THEME, VALUE)                                                         \
    {                                                                                     \
        const sce::Json::Value &a = VALUE.getValue(#A);                                   \
        THEME.style.A = {(float)a.getValue(0).getReal(), (float)a.getValue(1).getReal()}; \
    }
#define GET_DIR(A, THEME, VALUE) THEME.style.A = GetImGuiDir(VALUE.getValue(#A).getString().c_str());
#define GET_COLOR(A, COLORS, VALUE)                              \
    {                                                            \
        const sce::Json::Value &a = VALUE.getValue(#A);          \
        COLORS[ImGuiCol_##A] = {(float)a.getValue(0).getReal(),  \
                                (float)a.getValue(1).getReal(),  \
                                (float)a.getValue(2).getReal(),  \
                                (float)a.getValue(3).getReal()}; \
    }

    Theme theme;
    memcpy(&theme.style, &ImGui::GetStyle(), sizeof(ImGuiStyle));

    GET_STR(name, theme, value);

    const sce::Json::Value &styles = value.getValue("styles");

    GET_FLOAT(Alpha, theme, styles);
    GET_VEC2(WindowPadding, theme, styles);
    GET_FLOAT(WindowRounding, theme, styles);
    GET_FLOAT(WindowBorderSize, theme, styles);
    GET_VEC2(WindowMinSize, theme, styles);
    // GET_VEC2(WindowTitleAlign, theme, styles);
    GET_DIR(WindowMenuButtonPosition, theme, styles);
    GET_FLOAT(ChildRounding, theme, styles);
    GET_FLOAT(ChildBorderSize, theme, styles);
    GET_FLOAT(PopupRounding, theme, styles);
    GET_FLOAT(PopupBorderSize, theme, styles);
    GET_VEC2(FramePadding, theme, styles);
    GET_FLOAT(FrameRounding, theme, styles);
    GET_FLOAT(FrameBorderSize, theme, styles);
    GET_VEC2(ItemSpacing, theme, styles);
    GET_VEC2(ItemInnerSpacing, theme, styles);
    GET_FLOAT(IndentSpacing, theme, styles);
    GET_FLOAT(ColumnsMinSpacing, theme, styles);
    GET_FLOAT(ScrollbarSize, theme, styles);
    GET_FLOAT(ScrollbarRounding, theme, styles);
    GET_FLOAT(GrabMinSize, theme, styles);
    GET_FLOAT(GrabRounding, theme, styles);
    GET_FLOAT(TabRounding, theme, styles);
    GET_FLOAT(TabBorderSize, theme, styles);
    GET_FLOAT(TabMinWidthForUnselectedCloseButton, theme, styles);
    GET_DIR(ColorButtonPosition, theme, styles);
    GET_VEC2(ButtonTextAlign, theme, styles);
    GET_VEC2(SelectableTextAlign, theme, styles);

    ImVec4 *style_colors = theme.style.Colors;
    const sce::Json::Value &colors = value.getValue("colors");

    GET_COLOR(Text, style_colors, colors);
    GET_COLOR(TextDisabled, style_colors, colors);
    GET_COLOR(WindowBg, style_colors, colors);
    GET_COLOR(ChildBg, style_colors, colors);
    GET_COLOR(PopupBg, style_colors, colors);
    GET_COLOR(Border, style_colors, colors);
    GET_COLOR(BorderShadow, style_colors, colors);
    GET_COLOR(FrameBg, style_colors, colors);
    GET_COLOR(FrameBgHovered, style_colors, colors);
    GET_COLOR(FrameBgActive, style_colors, colors);
    GET_COLOR(TitleBg, style_colors, colors);
    GET_COLOR(TitleBgActive, style_colors, colors);
    GET_COLOR(TitleBgCollapsed, style_colors, colors);
    GET_COLOR(MenuBarBg, style_colors, colors);
    GET_COLOR(ScrollbarBg, style_colors, colors);
    GET_COLOR(ScrollbarGrab, style_colors, colors);
    GET_COLOR(ScrollbarGrabHovered, style_colors, colors);
    GET_COLOR(ScrollbarGrabActive, style_colors, colors);
    GET_COLOR(CheckMark, style_colors, colors);
    GET_COLOR(SliderGrab, style_colors, colors);
    GET_COLOR(SliderGrabActive, style_colors, colors);
    GET_COLOR(Button, style_colors, colors);
    GET_COLOR(ButtonHovered, style_colors, colors);
    GET_COLOR(ButtonActive, style_colors, colors);
    GET_COLOR(Header, style_colors, colors);
    GET_COLOR(HeaderHovered, style_colors, colors);
    GET_COLOR(HeaderActive, style_colors, colors);
    GET_COLOR(Separator, style_colors, colors);
    GET_COLOR(SeparatorHovered, style_colors, colors);
    GET_COLOR(SeparatorActive, style_colors, colors);
    GET_COLOR(ResizeGrip, style_colors, colors);
    GET_COLOR(ResizeGripHovered, style_colors, colors);
    GET_COLOR(ResizeGripActive, style_colors, colors);
    GET_COLOR(Tab, style_colors, colors);
    GET_COLOR(TabHovered, style_colors, colors);
    GET_COLOR(TabActive, style_colors, colors);
    GET_COLOR(TabUnfocused, style_colors, colors);
    GET_COLOR(TabUnfocusedActive, style_colors, colors);
    GET_COLOR(PlotLines, style_colors, colors);
    GET_COLOR(PlotLinesHovered, style_colors, colors);
    GET_COLOR(PlotHistogram, style_colors, colors);
    GET_COLOR(PlotHistogramHovered, style_colors, colors);
    GET_COLOR(TextSelectedBg, style_colors, colors);
    GET_COLOR(DragDropTarget, style_colors, colors);
    GET_COLOR(NavHighlight, style_colors, colors);
    GET_COLOR(NavWindowingHighlight, style_colors, colors);
    GET_COLOR(NavWindowingDimBg, style_colors, colors);
    GET_COLOR(ModalWindowDimBg, style_colors, colors);

    style_colors[ImGuiCol_TitleBg] = style_colors[ImGuiCol_TitleBgActive];
    ImVec4 c = style_colors[ImGuiCol_Tab];
    style_colors[ImGuiCol_Tab] = {std::clamp(c.x * 0.7f, 0.f, 1.0f),
                                  std::clamp(c.y * 0.7f, 0.f, 1.0f),
                                  std::clamp(c.z * 0.7f, 0.f, 1.0f),
                                  c.w};

    c = style_colors[ImGuiCol_TabActive];
    style_colors[ImGuiCol_TabActive] = {std::clamp(c.x * 1.2f, 0.f, 1.f),
                                        std::clamp(c.y * 1.2f, 0.f, 1.f),
                                        std::clamp(c.z * 1.2f, 0.f, 1.f),
                                        c.w};

    style_colors[ImGuiCol_PopupBg].w = std::clamp(style_colors[ImGuiCol_PopupBg].w, 0.9f, 1.f);

#define MIN_THRESHOLD 0.08

    if (std::abs(style_colors[ImGuiCol_PopupBg].x - style_colors[ImGuiCol_Header].x) < MIN_THRESHOLD &&
        std::abs(style_colors[ImGuiCol_PopupBg].y - style_colors[ImGuiCol_Header].y) < MIN_THRESHOLD &&
        std::abs(style_colors[ImGuiCol_PopupBg].z - style_colors[ImGuiCol_Header].z) < MIN_THRESHOLD)
        style_colors[ImGuiCol_Header] = style_colors[ImGuiCol_HeaderActive];

    return theme;
}

bool Themes::Load(const char *path)
{
    LogFunctionName;

    bool result = false;
    char *buf;
    size_t size = File::ReadFile(path, (void **)&buf);
    if (size == 0)
    {
        LogWarn("failed to read %s", path);
        return false;
    }

    sceSysmoduleLoadModule(SCE_SYSMODULE_JSON);

    {
        Utils::JsonAllocator allc;
        sce::Json::InitParameter params{&allc, nullptr, 0x400};
        sce::Json::Initializer init;
        sce::Json::Value root;

        if (init.initialize(&params) != SCE_OK)
        {
            LogWarn("failed to init json: %s", path);
            goto END;
        }

        if (sce::Json::Parser::parse(root, buf, size) != SCE_OK)
        {
            LogWarn("failed to parse json: %s", path);
            init.terminate();
            goto END;
        }

        sce::Json::Array array = root.getArray();
        _themes.reserve(array.size());
        for (const auto &value : array)
        {
            Theme &theme = _themes.emplace_back(GetTheme(value));
            LogDebug(theme.name.c_str());
        }
    }

END:
    sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON);
    delete[] buf;

    return result;
}

size_t Themes::GetIndexByName(const char *name) const
{
    size_t index = 0;
    size_t dark = 0;
    for (const auto &theme : _themes)
    {
        if (theme.name == name)
        {
            return index;
        }
        else if (theme.name == DEFAULT_THEME_NAME)
        {
            dark = index;
        }

        index++;
    }

    return dark;
}

void Themes::Apply(size_t index) const
{
    LogFunctionName;
    if (index >= 0 && index < _themes.size())
    {
        LogDebug("  theme: %s", _themes[index].name.c_str());

        ImGuiStyle &style = ImGui::GetStyle();
        const ImGuiStyle &theme_style = _themes[index].style;

        memcpy(&style, &theme_style, sizeof(ImGuiStyle));

        ImVec4 color = theme_style.Colors[ImGuiCol_Text];
        IM_COL32_GREEN = ImGui::GetColorU32({std::clamp(color.x * 0.1f, 0.f, 1.f),
                                             std::clamp(color.y * 1.5f, 0.3f, 1.f),
                                             std::clamp(color.z * 0.1f, 0.f, 1.f),
                                             color.w});
        IM_COL32_RED = ImGui::GetColorU32({std::clamp(color.x * 1.5f, 0.3f, 1.f),
                                           std::clamp(color.y * 0.1f, 0.f, 1.f),
                                           std::clamp(color.z * 0.1f, 0.f, 1.f),
                                           color.w});
        IM_COL32_YELLOW = ImGui::GetColorU32({std::clamp(color.x * 1.5f, 0.3f, 1.f),
                                              std::clamp(color.x * 1.5f, 0.3f, 1.f),
                                              std::clamp(color.z * 0.1f, 0.f, 1.f),
                                              color.w});
    }
}

void Themes::Apply(const char *name) const
{
    Apply(GetIndexByName(name));
}
