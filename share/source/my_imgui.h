#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <stdint.h>
#include "delay.h"

#define IM_COL32_GREEN IM_COL32(0, 255, 0, 255)
#define IM_COL32_RED IM_COL32(255, 0, 0, 255)
#define IM_COL32_DARK_GREY IM_COL32(100, 100, 100, 255)
#define DEFAULT_TEXT_MOVING_INTERVAL 15000
#define DEFAULT_TEXT_MOVING_START 800000

struct TextMovingStatus
{
    int pos = 0;
    int delta = -1;
    Delay<uint64_t> delay{DEFAULT_TEXT_MOVING_INTERVAL, DEFAULT_TEXT_MOVING_START};

    void Reset();
    bool Update(const char *text);
};

IMGUI_API void My_ImGui_ImplVita2D_Init(uint32_t language, const char *cache_path = NULL);
IMGUI_API void My_ImGui_ImplVita2D_Shutdown();
IMGUI_API void My_ImGui_ImplVita2D_RenderDrawData(ImDrawData *draw_data);
IMGUI_API void My_Imgui_Create_Font(uint32_t language, const char *cache_path = NULL);
IMGUI_API void My_Imgui_Destroy_Font(bool clean_fonts = true);

IMGUI_API bool My_ImGui_BeginCombo(const char *label, const char *preview_value, ImGuiComboFlags flags);
IMGUI_API void My_ImGui_CenteredText(const char *text, ...);
IMGUI_API bool My_ImGui_Selectable(const char *label, bool selected, TextMovingStatus *status);
IMGUI_API void My_ImGui_ShowTimePower(bool show_wifi = false, bool show_ra = false, bool ra_hardcore = false);
IMGUI_API void My_ImGui_HighlightText(const char *text, ImVec2 position, ImU32 text_color, ImU32 shadow_color);

class My_Imgui_SpinText
{
public:
    My_Imgui_SpinText(uint64_t interval_ms = 200000, ImU32 color = IM_COL32_GREEN);
    virtual ~My_Imgui_SpinText();
    void Show();

private:
    inline void _Update();

    static const char *_frames[];
    uint64_t _next_ms;
    uint64_t _interval_ms;
    size_t _count;
    ImU32 _color;
};
