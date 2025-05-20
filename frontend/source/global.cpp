#include <string.h>
#include <libretro.h>
#include <psp2common/ctrl.h>
#include "input.h"
#include "global.h"
#include "config_types.h"

AppStatus gStatus;
bool gBootFromArch = false;
std::string gBootRomPath;
const CORE_STRUCT *gCore = nullptr;

char CONSOLE_DIR[DEFINE_PATH_LENGTH];

char CORE_DATA_DIR[DEFINE_PATH_LENGTH];
char CORE_SAVEFILES_DIR[DEFINE_PATH_LENGTH];
char CORE_CHEATS_DIR[DEFINE_PATH_LENGTH];
char CORE_LOG_PATH[DEFINE_PATH_LENGTH];
char CORE_CONFIG_PATH[DEFINE_PATH_LENGTH];
char CORE_INPUT_DESC_PATH[DEFINE_PATH_LENGTH];
char CORE_FAVOURITE_PATH[DEFINE_PATH_LENGTH];

bool DEFAULT_ENABLE_REWIND = true;
size_t DEFAULT_REWIND_BUF_SIZE = 10;
bool DEFAULT_AUTO_SAVE = true;
bool DEFAULT_REBOOT_WHEN_LOADING_AGAIN = false;

bool DEFAULT_INDEPENDENT_CONFIG = false;

bool DEFAULT_LIGHTGUN = false;
bool DEFAULT_MOUSE = CONFIG_MOUSE_DISABLE;
bool ENABLE_KEYBOARD = false;

bool CONTROL_SPEED_BY_VIDEO = false;

std::vector<uint8_t> RETRO_KEYS;
std::vector<ControlMapConfig> CONTROL_MAPS;
std::vector<std::pair<const char *, const char *>> DEFAULT_CORE_SETTINGS;
std::vector<BIOS> REQUIRED_BIOS;

static void InitControlMapsAndRetroKeys()
{
    switch (gCore->console)
    {
    case GBA:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_B, true},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_A, true},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case ARC:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3, RETRO_DEVICE_ID_JOYPAD_L3},
            {SCE_CTRL_R3, RETRO_DEVICE_ID_JOYPAD_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_RSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_RSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_L2,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_L3,
            RETRO_DEVICE_ID_JOYPAD_R3,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case SNES:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case NES:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_A, true},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_B, true},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case GBC:
    case NGP:
    case WSC:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_A, true},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_B, true},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1},
            {SCE_CTRL_R1},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case MD:
    case PCE:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case PS1:
    case DOS:
    case NEOGEOCD:
    case PC98:
    case MSX:
    case C64:
    case X68000:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3, RETRO_DEVICE_ID_JOYPAD_L3},
            {SCE_CTRL_R3, RETRO_DEVICE_ID_JOYPAD_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_L2,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_L3,
            RETRO_DEVICE_ID_JOYPAD_R3,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case ATARI2600:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3, RETRO_DEVICE_ID_JOYPAD_L3},
            {SCE_CTRL_R3, RETRO_DEVICE_ID_JOYPAD_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_L2,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_L3,
            RETRO_DEVICE_ID_JOYPAD_R3,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case ATARI5200:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3, RETRO_DEVICE_ID_JOYPAD_L3},
            {SCE_CTRL_R3, RETRO_DEVICE_ID_JOYPAD_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_L2,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_L3,
            RETRO_DEVICE_ID_JOYPAD_R3,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case ATARI7800:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case VECTREX:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1},
            {SCE_CTRL_R1},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case AMIGA:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2, RETRO_DEVICE_ID_JOYPAD_L2},
            {SCE_CTRL_R2, RETRO_DEVICE_ID_JOYPAD_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_L2,
            RETRO_DEVICE_ID_JOYPAD_R2,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    case ZXS:
        CONTROL_MAPS = {
            {SCE_CTRL_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_CROSS, RETRO_DEVICE_ID_JOYPAD_B},
            {SCE_CTRL_TRIANGLE, RETRO_DEVICE_ID_JOYPAD_X},
            {SCE_CTRL_CIRCLE, RETRO_DEVICE_ID_JOYPAD_A},
            {SCE_CTRL_SQUARE, RETRO_DEVICE_ID_JOYPAD_Y},
            {SCE_CTRL_SELECT, RETRO_DEVICE_ID_JOYPAD_SELECT},
            {SCE_CTRL_START, RETRO_DEVICE_ID_JOYPAD_START},
            {SCE_CTRL_L1, RETRO_DEVICE_ID_JOYPAD_L},
            {SCE_CTRL_R1, RETRO_DEVICE_ID_JOYPAD_R},
            {SCE_CTRL_L2},
            {SCE_CTRL_R2},
            {SCE_CTRL_L3},
            {SCE_CTRL_R3},
            {SCE_CTRL_LSTICK_UP, RETRO_DEVICE_ID_JOYPAD_UP},
            {SCE_CTRL_LSTICK_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN},
            {SCE_CTRL_LSTICK_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT},
            {SCE_CTRL_LSTICK_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT},
            {SCE_CTRL_RSTICK_UP},
            {SCE_CTRL_RSTICK_DOWN},
            {SCE_CTRL_RSTICK_LEFT},
            {SCE_CTRL_RSTICK_RIGHT},
        };
        RETRO_KEYS = {
            RETRO_DEVICE_ID_NONE,
            RETRO_DEVICE_ID_JOYPAD_UP,
            RETRO_DEVICE_ID_JOYPAD_DOWN,
            RETRO_DEVICE_ID_JOYPAD_LEFT,
            RETRO_DEVICE_ID_JOYPAD_RIGHT,
            RETRO_DEVICE_ID_JOYPAD_A,
            RETRO_DEVICE_ID_JOYPAD_B,
            RETRO_DEVICE_ID_JOYPAD_X,
            RETRO_DEVICE_ID_JOYPAD_Y,
            RETRO_DEVICE_ID_JOYPAD_L,
            RETRO_DEVICE_ID_JOYPAD_R,
            RETRO_DEVICE_ID_JOYPAD_START,
            RETRO_DEVICE_ID_JOYPAD_SELECT,
        };
        break;
    }
}

static void InitDefaultCoreSettings()
{
    switch (gCore->core)
    {
    case DOSBOX_PURE:
        DEFAULT_CORE_SETTINGS = {{"dosbox_pure_cycles", "max"},
                                 {"dosbox_pure_force60fps", "true"}};
        break;
    case NEKOP2KAI:
        DEFAULT_CORE_SETTINGS = {{"np2kai_clk_mult", "2"},
                                 {"np2kai_joymode", "Keypad"}};
        break;
    case MAME2003:
        DEFAULT_CORE_SETTINGS = {{"mame2003_rstick_to_btns", "disabled"}};
        break;
    case MAME2003_XTREME:
        DEFAULT_CORE_SETTINGS = {{"mame2003-xtreme-amped-rstick_to_btns", "disabled"}};
        break;
    default:
        break;
    }
}

static void InitRequiredBIOS()
{
    switch (gCore->console)
    {
    case ATARI5200:
        REQUIRED_BIOS = {
            {"5200.rom", 0x4248d3e3},
            {"ATARIXL.ROM", 0x1f9cd270},
            {"ATARIBAS.ROM", 0x7d684184},
            {"ATARIOSA.ROM", 0x72b3fed4},
            {"ATARIOSB.ROM", 0x3e28a1fe},
        };
        break;
    case ATARI7800:
        REQUIRED_BIOS = {{"7800 BIOS (U).rom", 0x5d13730c}};
        break;
    case ZXS:
        REQUIRED_BIOS = {
            {"fuse/128p-0.rom", 0},
            {"fuse/128p-1.rom", 0},
            {"fuse/trdos.rom", 0},
            {"fuse/gluck.rom", 0},
            {"fuse/256s-0.rom", 0},
            {"fuse/256s-1.rom", 0},
            {"fuse/256s-2.rom", 0},
            {"fuse/256s-3.rom", 0},
        };
        break;
    case MSX:
        REQUIRED_BIOS = {
            {"MSX.ROM", 0},
            {"MSX2.ROM", 0},
            {"MSX2EXT.ROM", 0},
            {"MSX2P.ROM", 0},
            {"MSX2PEXT.ROM", 0},
            {"DISK.ROM", 0},
            {"FMPAC.ROM", 0},
            {"MSXDOS2.ROM", 0},
            {"PAINTER.ROM", 0},
            {"KANJI.ROM", 0},
        };
        break;
    case NES:
        REQUIRED_BIOS = {{"disksys.rom", 0x5e607dcf}};
        break;
    case PCE:
        REQUIRED_BIOS = {{"syscard3.pce", 0}};
        break;
    case GBC:
        REQUIRED_BIOS = {
            {"gb_bios.bin", 0},
            {"gbc_bios.bin", 0},
        };
        break;
    case GBA:
        REQUIRED_BIOS = {{"gba_bios.bin", 0}};
        break;
    case NEOGEOCD:
        REQUIRED_BIOS = {
            {"neocd/neocd_f.rom", 0},
            {"neocd/neocd_sf.rom", 0},
            {"neocd/neocd_t.rom", 0},
            {"neocd/neocd_st.rom", 0},
            {"neocd/neocd_z.rom", 0},
            {"neocd/neocd_sz.rom", 0},
            {"neocd/front-sp1.bin", 0},
            {"neocd/top-sp1.bin", 0},
            {"neocd/neocd.bin", 0},
            {"neocd/uni-bioscd.rom", 0},
        };
        break;
    case PS1:
        REQUIRED_BIOS = {
            {"PSXONPSP660.bin", 0},
            {"scph101.bin", 0},
            {"scph7001.bin", 0},
            {"scph5501.bin", 0},
            {"scph1001.bin", 0},
        };
        break;
    case PC98:
        if (gCore->core == NEKOP2)
        {
            REQUIRED_BIOS = {
                {"np2/font.bmp", 0},
                {"np2/FONT.ROM", 0},
                {"np2/bios.rom", 0},
                {"np2/itf.rom", 0},
                {"np2/sound.rom", 0},
                {"np2/bios9821.rom", 0},
                {"np2/d8000.rom", 0},
                {"np2/2608_BD.WAV", 0},
                {"np2/2608_SD.WAV", 0},
                {"np2/2608_TOP.WAV", 0},
                {"np2/2608_HH.WAV", 0},
                {"np2/2608_TOM.WAV", 0},
                {"np2/2608_RIM.WAV", 0},
            };
        }
        else // NEKOP2KAI
        {
            REQUIRED_BIOS = {
                {"np2kai/font.bmp", 0},
                {"np2kai/FONT.ROM", 0},
                {"np2kai/bios.rom", 0},
                {"np2kai/itf.rom", 0},
                {"np2kai/sound.rom", 0},
                {"np2kai/bios9821.rom", 0},
                {"np2kai/d8000.rom", 0},
                {"np2kai/2608_BD.WAV", 0},
                {"np2kai/2608_SD.WAV", 0},
                {"np2kai/2608_TOP.WAV", 0},
                {"np2kai/2608_HH.WAV", 0},
                {"np2kai/2608_TOM.WAV", 0},
                {"np2kai/2608_RIM.WAV", 0},
            };
        }
        break;
    case C64:
        REQUIRED_BIOS = {
            {"vice/JiffyDOS_C64.bin", 0},
            {"vice/JiffyDOS_1541-II.bin", 0},
            {"vice/JiffyDOS_1571_repl310654.bin", 0},
            {"vice/JiffyDOS_1581.bin", 0},
            {"vice/scpu-dos-1.4.bin", 0},
            {"vice/scpu-dos-2.04.bin", 0},
            {"vice/SCPU64/JiffyDOS_C128.bin", 0},
        };
        break;
    case X68000:
        REQUIRED_BIOS = {
            {"keropi/iplrom.dat", 0},
            {"keropi/cgrom.dat", 0},
        };
        break;
    default:
        break;
    }
}

bool InitDefines()
{
    for (const auto &c : CORES)
    {
        if (strcmp(c.core_name, CORE_NAME) == 0)
        {
            gCore = &c;
            break;
        }
    }

    if (gCore == nullptr)
    {
        return false;
    }

    snprintf(CONSOLE_DIR, DEFINE_PATH_LENGTH, "app0:data/%s", gCore->console_name);

    snprintf(CORE_DATA_DIR, DEFINE_PATH_LENGTH, ROOT_DIR "/%s", gCore->core_short_name);
    snprintf(CORE_SAVEFILES_DIR, DEFINE_PATH_LENGTH, "%s/savefiles", CORE_DATA_DIR);
    snprintf(CORE_CHEATS_DIR, DEFINE_PATH_LENGTH, "%s/cheats", CORE_DATA_DIR);

    snprintf(CORE_LOG_PATH, DEFINE_PATH_LENGTH, "%s/Emu4Vita++.log", CORE_DATA_DIR);
    snprintf(CORE_CONFIG_PATH, DEFINE_PATH_LENGTH, "%s/config.ini", CORE_DATA_DIR);
    snprintf(CORE_INPUT_DESC_PATH, DEFINE_PATH_LENGTH, "%s/input_desc.ini", CORE_DATA_DIR);
    snprintf(CORE_FAVOURITE_PATH, DEFINE_PATH_LENGTH, "%s/favourite.ini", CORE_DATA_DIR);

    if (gCore->console & (ARC | AMIGA | ATARI2600 | ATARI5200 | ATARI7800 | ZXS | PC98 | MSX | C64 | X68000))
    {
        DEFAULT_ENABLE_REWIND = false;
        DEFAULT_REWIND_BUF_SIZE = 50;
        DEFAULT_AUTO_SAVE = false;
        // DEFAULT_REBOOT_WHEN_LOADING_AGAIN = true;
    }

    if (gCore->console == ARC)
    {
        DEFAULT_INDEPENDENT_CONFIG = true;
    }

    if (gCore->console & (NES | MD | SNES))
    {
        DEFAULT_LIGHTGUN = true;
    }

    if (gCore->console & (SNES | DOS))
    {
        DEFAULT_MOUSE = CONFIG_MOUSE_REAR;
    }

    if (gCore->console & (DOS | AMIGA | ZXS | PC98 | MSX | C64 | X68000 | ATARI5200))
    {
        ENABLE_KEYBOARD = true;
    }

    if (gCore->core == VBA_NEXT)
    {
        CONTROL_SPEED_BY_VIDEO = true;
    }

    InitControlMapsAndRetroKeys();
    InitDefaultCoreSettings();

    return true;
}
