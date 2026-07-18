#include "time_scale.h"
#include "defines.h"
#include "log.h"

#define SCALE_TEXTURE_PATH (APP_ASSETS_DIR "/time_scale.png")
#define YEAR_PIXEL_STEP 30
#define START_YEAR 1965

TimeScale::TimeScale() : _count(0)
{
    _scale_texture = vita2d_load_PNG_file(SCALE_TEXTURE_PATH);
    _width = vita2d_texture_get_width(_scale_texture);
    *_year = 0;
    _x[0] = 0.f;
    _year_x[0] = 0.f;
}

TimeScale::~TimeScale()
{
    vita2d_free_texture(_scale_texture);
}

void TimeScale::Show()
{
    ImVec4 image_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    image_color.w *= 2.f;
    image_color.w /= 3.f;
    ImGui::Image(_scale_texture,
                 {TIME_SCALE_WIDTH, 50},
                 {_x[_count], 0.f},
                 {_x[_count] + TIME_SCALE_WIDTH / _width, 1.f},
                 image_color);

    ImVec2 size = ImGui::CalcTextSize(_year);
    if (size.x > 0)
    {
        ImGui::SetCursorPos({_year_x[_count] - size.x / 2.f, 25.f});
        ImGui::TextUnformatted(_year);
    }

    if (_count > 0)
        _count--;
}

static void _GetSteps(float start, float end, float *out)
{
    const float delta = end - start;

    for (int i = 0; i < GRADIENT_FRAMES; ++i)
    {
        float t = (float)(GRADIENT_FRAMES - 1 - i) / (float)(GRADIENT_FRAMES - 1); // 1 -> 0
        float inv = 1.0f - t;
        float k = 1.0f - inv * inv; // EaseOutCubic

        out[i] = start + delta * k;
        // LogDebug("%d %f", i, out[i]);
    }
}

void TimeScale::SetTime(int year, int offset)
{
    LogFunctionName;
    LogDebug("  %d %d", year, offset);
    if (year == 0)
    {
        *_year = 0;
    }
    else
    {
        snprintf(_year, 8, "%d", year);
        _GetSteps(_x[0], ((year - START_YEAR) * YEAR_PIXEL_STEP - offset) / _width, _x);
        _GetSteps(_year_x[0], offset, _year_x);
        _count = GRADIENT_FRAMES - 1;
    }
}