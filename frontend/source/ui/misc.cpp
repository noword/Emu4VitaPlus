#include <string>
#include <string.h>
#include <zlib.h>
#include "misc.h"
#include "defines.h"
#include "file.h"
#include "state_manager.h"
#include "log.h"
#include "arcade_manager.h"
#include "global.h"
#include "network.h"

vita2d_texture *GetRomPreviewImage(const char *path, const char *name, const char *english_name, bool include_state)
{
    LogFunctionName;
    LogDebug("%s %s", path, name);
    vita2d_texture *texture = nullptr;

    std::string stem = File::GetStem(name);
    std::string img_path = std::string(path) + "/" PREVIEW_DIR_NAME "/" + stem;

    texture = vita2d_load_PNG_file((img_path + ".png").c_str());
    if (texture)
        goto END;

    texture = vita2d_load_JPEG_file((img_path + ".jpg").c_str());
    if (texture)
        goto END;

    if (english_name && *english_name && gConfig->auto_download_thumbnail)
    {
        img_path = std::string(THUMBNAILS_PATH) + '/' + english_name + ".png";
        texture = vita2d_load_PNG_file((img_path).c_str());
        if (texture)
            goto END;

        int count = 0;
        std::string english = gNetwork->Escape(english_name);
        while (THUMBNAILS_NAME[count] != nullptr)
        {
            std::string url = std::string(LIBRETRO_THUMBNAILS) + THUMBNAILS_NAME[count++] + "/" THUMBNAILS_SUBDIR "/" + english + ".png";
            if (gNetwork->Download(url.c_str(), img_path.c_str()))
            {
                texture = vita2d_load_PNG_file((img_path).c_str());
                if (texture)
                    goto END;
            }
        }
    }

    if (include_state)
    {
        img_path = std::string(CORE_SAVEFILES_DIR) + '/' + stem + '/';
        time_t newest;
        for (int i = 0; i < MAX_STATES; i++)
        {
            char tmp[8];
            if (i == 0)
            {
                strcpy(tmp, "auto");
            }
            else
            {
                snprintf(tmp, 8, "%02d", i);
            }
            std::string jpg_path = img_path + "state_" + tmp + ".jpg";
            if (File::Exist(jpg_path.c_str()))
            {
                if (texture == nullptr)
                {
                    texture = vita2d_load_JPEG_file(jpg_path.c_str());
                    File::GetCreateTime(jpg_path.c_str(), &newest);
                }
                else
                {
                    time_t time;
                    File::GetCreateTime(jpg_path.c_str(), &time);
                    if (time > newest)
                    {
                        vita2d_free_texture(texture);
                        texture = vita2d_load_JPEG_file(jpg_path.c_str());
                        newest = time;
                    }
                }
            }
        }
    }

END:
    return texture;
}

void CalcFitSize(float width, float height, float max_width, float max_height, float *out_width, float *out_height)
{
    *out_height = height * max_width / width;
    if (*out_height > max_height)
    {
        *out_height = max_height;
        *out_width = width * max_height / height;
    }
    else
    {
        *out_width = max_width;
    }
}

static void FormatNumber(unsigned int v, char *s)
{
    char s_temp[32];
    sprintf(s_temp, "%u", v);
    int len = strlen(s_temp);

    int remainder = len % 3;
    if (remainder == 0 && len != 0)
    {
        remainder = 3;
    }

    char *dst = s;
    const char *src = s_temp;

    for (int i = 0; i < remainder; ++i)
    {
        *dst++ = *src++;
    }

    int remaining = len - remainder;
    if (remaining > 0)
    {
        *dst++ = ',';
        int num_groups = remaining / 3;
        for (int i = 0; i < num_groups; ++i)
        {
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            if (i != num_groups - 1)
            {
                *dst++ = ',';
            }
        }
    }

    *dst = '\0';
}

std::string GetFileInfoString(const char *path)
{
    char s[64];
    SceDateTime time;
    size_t size = File::GetSize(path);
    if (size == 0 || !File::GetCreateTime(path, &time))
    {
        return "";
    }

    char num[64];
    FormatNumber(size, num);
    snprintf(s, 64, "  %d/%02d/%02d  %02d:%02d %20s", time.year < 100 ? time.year + 1969 : time.year, time.month, time.day, time.hour, time.minute, num);

    return s;
}
