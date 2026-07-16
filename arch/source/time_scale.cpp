#include "time_scale.h"
#include "defines.h"
#include "log.h"

#define SCALE_TEXTURE_PATH (APP_ASSETS_DIR "/time_scale.png")
#define YEAR_PIXEL_STEP 30
#define START_YEAR 1965

TimeScale::TimeScale() : _x(0.f)
{
    _scale_texture = vita2d_load_PNG_file(SCALE_TEXTURE_PATH);
    _width = vita2d_texture_get_width(_scale_texture);
    *_year = 0;
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
                 {_x, 0.f},
                 {_x + TIME_SCALE_WIDTH / _width, 1.f},
                 image_color);

    ImVec2 size = ImGui::CalcTextSize(_year);
    if (size.x > 0)
    {
        ImGui::SetCursorPos({_year_offset - size.x / 2.f, 25.f});
        ImGui::TextUnformatted(_year);
    }
}

void TimeScale::SetTime(int year, int offset)
{
    if (year == 0)
    {
        *_year = 0;
    }
    else
    {
        snprintf(_year, 8, "%d", year);
        _year_offset = offset;
        _x = ((year - START_YEAR) * YEAR_PIXEL_STEP - offset) / _width;
    }
}