#include <time.h>
#include <stdlib.h>
#include "tab_about.h"
#include "language_string.h"
#include "my_imgui.h"
#include "defines.h"
#include "utils.h"
#include "global.h"

#define TITLE_COUNT 7
#define TITLE_WIDTH 940
#define TITLE_HEIGHT 100
#define INPUT_COUNT_MAX 10

TabAbout::TabAbout() : TabBase(LANG_ABOUT),
                       _index(0),
                       _input_count(0)
{
    _InitTexts();
    _title_texture = vita2d_load_PNG_file("app0:assets/emu4vita++.png");

    time_t t;
    srand((unsigned)time(&t));
    _title_index = rand() % TITLE_COUNT;
}

TabAbout::~TabAbout()
{
}

void TabAbout::_Show()
{
    if (_last_lang != gConfig->language)
    {
        _InitTexts();
    }

    if (_input_count > INPUT_COUNT_MAX)
    {
        _input_count = 0;
        int old = _title_index;
        int index;
        while ((index = rand() % TITLE_COUNT) == old)
        {
        }
        _title_index = index;
    }

    if (_title_texture != nullptr)
    {
        ImGui::Image(_title_texture,
                     {TITLE_WIDTH, TITLE_HEIGHT},
                     {0, _title_index / TITLE_COUNT},
                     {1, (_title_index + 1) / TITLE_COUNT});
    }

    if (ImGui::BeginChild("ChildAbout", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        for (size_t i = 0; i < _texts.size(); i++)
        {
            My_ImGui_CenteredText(_texts[i].c_str());
            if (i == _index && ImGui::GetScrollMaxY() > 0.f)
            {
                ImGui::SetScrollHereY((float)_index / _texts.size());
            }
        }
    }

    ImGui::EndChild();
}

void TabAbout::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&TabAbout::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&TabAbout::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_UP, std::bind(&TabAbout::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_DOWN, std::bind(&TabAbout::_OnKeyDown, this, input), true);
}

void TabAbout::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_DOWN);
}

static const char *LIBRARIES_URL[] = {
    "7-Zip (GPL v2.1) https://github.com/mcmilk/7-Zip",
    "zlib-ng (zlib) https://github.com/zlib-ng/zlib-ng",
    "minizip-ng (zlib) https://github.com/zlib-ng/minizip-ng",
    "simpleini (MIT) https://github.com/brofield/simpleini",
    "libretro-common (?) https://github.com/libretro/libretro-common",
    "libvita2d (MIT) https://github.com/xerpi/libvita2d",
    "libvita2d_ext (MIT) https://github.com/frangarcj/libvita2d_ext",
    "imgui-vita2d (MIT) https://github.com/cy33hc/imgui-vita2d",
    "lz4 (BSD 2-Clause) https://github.com/lz4/lz4",
    ""};

static const char *CORES_URL[] = {
    "https://gitee.com/yizhigai/libretro-fba-lite",
    "https://github.com/libretro/FBNeo",
    "https://github.com/libretro/fbalpha2012",
    "https://github.com/libretro/mame2000-libretro",
    "https://github.com/libretro/mame2003-libretro",
    "https://github.com/libretro/mame2003-plus-libretro",
    "https://github.com/libretro/gpsp",
    "https://github.com/libretro/libretro-fceumm",
    "https://github.com/libretro/gambatte-libretro",
    "https://github.com/libretro/beetle-ngp-libretro",
    "https://github.com/libretro/beetle-pce-fast-libretro",
    "https://github.com/libretro/Genesis-Plus-GX",
    "https://github.com/libretro/Genesis-Plus-GX-Wide",
    "https://github.com/libretro/beetle-supergrafx-libretro",
    "https://github.com/libretro/beetle-wswan-libretro",
    "https://github.com/libretro/nestopia",
    "https://github.com/libretro/pcsx_rearmed",
    "https://github.com/libretro/picodrive",
    "https://github.com/libretro/snes9x2002",
    "https://github.com/libretro/snes9x2005",
    "https://github.com/libretro/snes9x",
    "https://github.com/libretro/vba-next",
    "https://github.com/libretro/mgba",
    "https://github.com/libretro/dosbox-pure",
    "https://github.com/KMFDManic/FBNeo-Xtreme-Amped",
    "https://github.com/KMFDManic/mame2003-xtreme",
    "https://github.com/Rinnegatamante/supafaust",
    "https://github.com/jamsilva/chimerasnes",
    "https://github.com/libretro/neocd_libretro",
    "https://github.com/libretro/stella2014-libretro",
    "https://github.com/libretro/libretro-atari800",
    "https://github.com/libretro/prosystem-libretro",
    "https://github.com/libretro/fuse-libretro",
    "https://github.com/libretro/neocd_libretro",
    "https://github.com/KMFDManic/mame2003-xtreme",
    "https://github.com/KMFDManic/FBNeo-Xtreme-Amped",
    "https://github.com/Rinnegatamante/supafaust",
    "https://github.com/jamsilva/chimerasnes",
    "https://github.com/libretro/tgbdual-libretro",
    "https://github.com/libretro/libretro-meowPC98",
    "https://github.com/libretro/blueMSX-libretro",
    "https://github.com/libretro/fmsx-libretro",
    "https://github.com/libretro/vice-libretro",
    "https://github.com/libretro/px68k-libretro"};

void TabAbout::_InitTexts()
{
    _last_lang = gConfig->language;
    _texts = {std::string(TEXT(LANG_COMPILED_ON)) + " " + __DATE__ + " " + __TIME__,
              std::string(TEXT(LANG_AUTHOR)) + ": noword",
              std::string(TEXT(LANG_SPECIAL_THANKS)) + ": yizhigai, KyleBing, TearCrow",
              std::string(TEXT(LANG_RETROACHIEVEMENTS_TESTER)) + ": Scott",
              std::string(TEXT(LANG_JAPANESE_TRANS)) + ": yyoossk",
              std::string(TEXT(LANG_ITALIAN_TRANS)) + ": theheroGAC",
              std::string(TEXT(LANG_FRENCH_TRANS)) + ": chronoss09",
              std::string(TEXT(LANG_SPANISH_TRANS)) + ": limonetas",
              std::string(TEXT(LANG_RUSSIAN_TRANS)) + ": MayanKoyote",
              std::string(TEXT(LANG_SITE)) + ": https://github.com/noword/Emu4VitaPlus",
              "",
              TEXT(LANG_EXTERNAL_COMPONENTS)};

    _texts.reserve(_texts.size() + sizeof(LIBRARIES_URL) / sizeof(LIBRARIES_URL[0]) + sizeof(CORES_URL) / sizeof(CORES_URL[0]));

    for (const auto &lib : LIBRARIES_URL)
    {
        _texts.emplace_back(lib);
    }

    _texts.push_back(TEXT(LANG_CORE));

    for (const auto &core : CORES_URL)
    {
        _texts.emplace_back(core);
    }
};

void TabAbout::_OnKeyUp(Input *input)
{
    if (_index > 0)
    {
        _index--;
    }
    _input_count++;
}

void TabAbout::_OnKeyDown(Input *input)
{
    if (_index + 1 < _texts.size())
    {
        _index++;
    }
    _input_count++;
}