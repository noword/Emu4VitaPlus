#pragma once
#include <vita2d.h>
#include <string>

#define MAIN_WINDOW_PADDING 10
#define BROWSER_TEXTURE_MAX_WIDTH 446
#define BROWSER_TEXTURE_MAX_HEIGHT 394

vita2d_texture *GetRomPreviewImage(const char *path, const char *name, const char *rom_name, bool include_state = true);
void CalcFitSize(float width, float height, float max_width, float max_height, float *out_width, float *out_height);
std::string GetFileInfoString(const char *path);
