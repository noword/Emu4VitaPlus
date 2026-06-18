// TEst
#include "libretro.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "crocods-core/platform.h"
#include "crocods-core/gif.h"

int Sound = 1;

extern const unsigned char icons_gif[];
extern const unsigned char cpc6128_bin[];
extern const unsigned char romdisc_bin[];

#define maxByCycle 400 // 50 fois par frame

char Core_Key_Sate[512];
char Core_old_Key_Sate[512];

static int KeySymToCPCKey[RETROK_LAST];

retro_log_printf_t log_cb;
retro_video_refresh_t video_cb;

static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

struct CrocoKeyMap {
    unsigned port;
    unsigned index;

    CPC_SCANCODE scanCode;
} crocokeymap[] = {
    {0, RETRO_DEVICE_ID_JOYPAD_A,      CPC_JOY_FIRE1     },
    {0, RETRO_DEVICE_ID_JOYPAD_B,      CPC_JOY_FIRE2     },
    {0, RETRO_DEVICE_ID_JOYPAD_UP,     CPC_JOY_UP        },
    {0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  CPC_JOY_RIGHT     },
    {0, RETRO_DEVICE_ID_JOYPAD_LEFT,   CPC_JOY_LEFT      },
    {0, RETRO_DEVICE_ID_JOYPAD_DOWN,   CPC_JOY_DOWN      },
    {0, RETRO_DEVICE_ID_JOYPAD_X,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_Y,      CPC_NIL           }, // 7
    {0, RETRO_DEVICE_ID_JOYPAD_L,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_R,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE         },
    {0, RETRO_DEVICE_ID_JOYPAD_START,  CPC_RETURN        }, // 11

    {1, RETRO_DEVICE_ID_JOYPAD_A,      CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_B,      CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_UP,     CPC_CURSOR_UP     },
    {1, RETRO_DEVICE_ID_JOYPAD_RIGHT,  CPC_CURSOR_RIGHT  },
    {1, RETRO_DEVICE_ID_JOYPAD_LEFT,   CPC_CURSOR_LEFT   },
    {1, RETRO_DEVICE_ID_JOYPAD_DOWN,   CPC_CURSOR_DOWN   },
    {1, RETRO_DEVICE_ID_JOYPAD_X,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_Y,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_L,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_R,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_START,  CPC_RETURN        }

};

// Crocods variable

core_crocods_t gb;

u8 *disk = NULL;
u32 diskLength;

u8 *snapshot = NULL;
u32 snapshotLength;

char autoString[256];

u16 scanlineMask[] = {0b1110111101011101,
                      0b1110011100011100,
                      0b1100011000011000,
                      0b1000010000010000};

u16 textureBytes[384 * 288 * 2];

u32 old_width1 = 0, old_height1 = 0, old_left1 = 0, old_top1 = 0, old_bpl1 = 0;
u16 old_width2 = 0, old_height2 = 0;

u32 *incX, *incY;

// end of crocods variable

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    va_list va;

    (void)level;

    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

static void updateFromEnvironment(void)
{
    // struct retro_variable pk1var = {"crocods_greenmonitor"};

    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk1var) && pk1var.value) {
    //     if (!strcmp(pk1var.value, "green")) {
    //         ExecuteMenu(&gb, ID_GREEN_MONITOR, NULL);
    //     } else if (!strcmp(pk1var.value, "color")) {
    //         ExecuteMenu(&gb, ID_COLOR_MONITOR, NULL);
    //     }
    // }

    // struct retro_variable pk2var = {"crocods_resize"};
    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk2var) && pk2var.value) {
    //     if (!strcmp(pk2var.value, "Auto")) {
    //         ExecuteMenu(&gb, ID_SCREEN_AUTO, NULL);
    //     } else if (!strcmp(pk2var.value, "320x200")) {
    //         ExecuteMenu(&gb, ID_SCREEN_320, NULL);
    //     } else if (!strcmp(pk2var.value, "Overscan")) {
    //         ExecuteMenu(&gb, ID_SCREEN_OVERSCAN, NULL);
    //     }
    // }

    // struct retro_variable pk3var = {"crocods_hack"};
    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk3var) && pk3var.value) {
    //     if (!strcmp(pk3var.value, "no")) {
    //         ExecuteMenu(&gb, ID_NOHACK_TABCOUL, NULL);
    //     } else if (!strcmp(pk3var.value, "yes")) {
    //         ExecuteMenu(&gb, ID_HACK_TABCOUL, NULL);
    //     }
    // }

}

void retro_init(void)
{
    char *savedir = NULL;
    int i;
    int bx, by;

    // Set Video
    incX = (u32 *)malloc(384 * 2 * sizeof(u32)); // malloc the max width
    incY = (u32 *)malloc(272 * sizeof(u32));     // malloc the max height

    environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir);


    char oldOpenFilename[MAX_PATH+1];
    strcpy(oldOpenFilename, gb.openFilename);

    memset(&gb, 0, sizeof(gb));

    strcpy(gb.openFilename, oldOpenFilename);

    // Get map layout

    // gb.keyboardLayout = 1; // French
	gb.keyboardLayout = 0;
	
    for (i = 0; i < RETROK_LAST; i++)
        KeySymToCPCKey[i] = CPC_NIL;

    /* International key mappings */
    KeySymToCPCKey[RETROK_0] = CPC_ZERO;
    KeySymToCPCKey[RETROK_1] = CPC_1;
    KeySymToCPCKey[RETROK_2] = CPC_2;
    KeySymToCPCKey[RETROK_3] = CPC_3;
    KeySymToCPCKey[RETROK_4] = CPC_4;
    KeySymToCPCKey[RETROK_5] = CPC_5;
    KeySymToCPCKey[RETROK_6] = CPC_6;
    KeySymToCPCKey[RETROK_7] = CPC_7;
    KeySymToCPCKey[RETROK_8] = CPC_8;
    KeySymToCPCKey[RETROK_9] = CPC_9;
    KeySymToCPCKey[RETROK_a] = CPC_A;
    KeySymToCPCKey[RETROK_b] = CPC_B;
    KeySymToCPCKey[RETROK_c] = CPC_C;
    KeySymToCPCKey[RETROK_d] = CPC_D;
    KeySymToCPCKey[RETROK_e] = CPC_E;
    KeySymToCPCKey[RETROK_f] = CPC_F;
    KeySymToCPCKey[RETROK_g] = CPC_G;
    KeySymToCPCKey[RETROK_h] = CPC_H;
    KeySymToCPCKey[RETROK_i] = CPC_I;
    KeySymToCPCKey[RETROK_j] = CPC_J;
    KeySymToCPCKey[RETROK_k] = CPC_K;
    KeySymToCPCKey[RETROK_l] = CPC_L;
    KeySymToCPCKey[RETROK_m] = CPC_M;
    KeySymToCPCKey[RETROK_n] = CPC_N;
    KeySymToCPCKey[RETROK_o] = CPC_O;
    KeySymToCPCKey[RETROK_p] = CPC_P;
    KeySymToCPCKey[RETROK_q] = CPC_Q;
    KeySymToCPCKey[RETROK_r] = CPC_R;
    KeySymToCPCKey[RETROK_s] = CPC_S;
    KeySymToCPCKey[RETROK_t] = CPC_T;
    KeySymToCPCKey[RETROK_u] = CPC_U;
    KeySymToCPCKey[RETROK_v] = CPC_V;
    KeySymToCPCKey[RETROK_w] = CPC_W;
    KeySymToCPCKey[RETROK_x] = CPC_X;
    KeySymToCPCKey[RETROK_y] = CPC_Y;
    KeySymToCPCKey[RETROK_z] = CPC_Z;
    KeySymToCPCKey[RETROK_SPACE] = CPC_SPACE;
    KeySymToCPCKey[RETROK_COMMA] = CPC_COMMA;
    KeySymToCPCKey[RETROK_PERIOD] = CPC_DOT;
    KeySymToCPCKey[RETROK_SEMICOLON] = CPC_COLON;
    KeySymToCPCKey[RETROK_MINUS] = CPC_MINUS;
    KeySymToCPCKey[RETROK_EQUALS] = CPC_HAT;
    KeySymToCPCKey[RETROK_LEFTBRACKET] = CPC_AT;
    KeySymToCPCKey[RETROK_RIGHTBRACKET] = CPC_OPEN_SQUARE_BRACKET;

    KeySymToCPCKey[RETROK_TAB] = CPC_TAB;
    KeySymToCPCKey[RETROK_RETURN] = CPC_RETURN;
    KeySymToCPCKey[RETROK_BACKSPACE] = CPC_DEL;
    KeySymToCPCKey[RETROK_ESCAPE] = CPC_ESC;

    // KeySymToCPCKey[RETROK_Equals & 0x0ff)] = CPC_CLR;

    KeySymToCPCKey[RETROK_UP] = CPC_CURSOR_UP;
    KeySymToCPCKey[RETROK_DOWN] = CPC_CURSOR_DOWN;
    KeySymToCPCKey[RETROK_LEFT] = CPC_CURSOR_LEFT;
    KeySymToCPCKey[RETROK_RIGHT] = CPC_CURSOR_RIGHT;

    KeySymToCPCKey[RETROK_KP0] = CPC_F0;
    KeySymToCPCKey[RETROK_KP1] = CPC_F1;
    KeySymToCPCKey[RETROK_KP2] = CPC_F2;
    KeySymToCPCKey[RETROK_KP3] = CPC_F3;
    KeySymToCPCKey[RETROK_KP4] = CPC_F4;
    KeySymToCPCKey[RETROK_KP5] = CPC_F5;
    KeySymToCPCKey[RETROK_KP6] = CPC_F6;
    KeySymToCPCKey[RETROK_KP7] = CPC_F7;
    KeySymToCPCKey[RETROK_KP8] = CPC_F8;
    KeySymToCPCKey[RETROK_KP9] = CPC_F9;

    KeySymToCPCKey[RETROK_KP_PERIOD] = CPC_FDOT;

    KeySymToCPCKey[RETROK_LSHIFT] = CPC_SHIFT;
    KeySymToCPCKey[RETROK_RSHIFT] = CPC_SHIFT;
    KeySymToCPCKey[RETROK_LCTRL] = CPC_CONTROL;
    KeySymToCPCKey[RETROK_RCTRL] = CPC_CONTROL;
    KeySymToCPCKey[RETROK_CAPSLOCK] = CPC_CAPS_LOCK;

    KeySymToCPCKey[RETROK_KP_ENTER] = CPC_SMALL_ENTER;

    KeySymToCPCKey[RETROK_DELETE] = CPC_JOY_LEFT;
    KeySymToCPCKey[RETROK_END] = CPC_JOY_DOWN;
    KeySymToCPCKey[RETROK_PAGEDOWN] = CPC_JOY_RIGHT;
    KeySymToCPCKey[RETROK_INSERT] = CPC_JOY_FIRE1;
    KeySymToCPCKey[RETROK_HOME] = CPC_JOY_UP;
    KeySymToCPCKey[RETROK_PAGEUP] = CPC_JOY_FIRE2;

    KeySymToCPCKey[0x0134] = CPC_COPY; /* Alt */
    KeySymToCPCKey[0x0137] = CPC_COPY; /* Compose */

    // Init nds

    nds_initBorder(&gb, &bx, &by);
    nds_init(&gb);

    updateFromEnvironment();

    AutoType_Init(&gb);

    Sound = 1;
    HardResetCPC(&gb);

    initSound(&gb, 44100);
}

void retro_deinit(void)
{

}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;


}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "crocods";
    info->need_fullpath = false;
    info->valid_extensions = "sna|dsk|kcr";

#ifdef GIT_VERSION
    info->library_version = "git" GIT_VERSION;
#else
    info->library_version = "svn";
#endif

}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->timing.fps = 50.0;
    info->timing.sample_rate = 44100.0;

    info->geometry.base_width = 384 * 2;
    info->geometry.base_height = 288;

    info->geometry.max_width = 384 * 2;
    info->geometry.max_height = 288;
    info->geometry.aspect_ratio = 1;
}

void retro_set_environment(retro_environment_t cb)
{
    struct retro_log_callback logging;

    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    log_cb = fallback_log;


    static const struct retro_variable vars[] = {
        // { "crocods_computertype", "Machine Type (Restart); CPC 464|CPC 6128" },
        // { "crocods_vdpsync", "VDP Sync Type (Restart); Auto|50Hz|60Hz" },
        // {"crocods_greenmonitor", "Color Monitor; color|green"         },
        // {"crocods_resize",       "Resize; Auto|320x200|Overscan"      },
        // {"crocods_hack",         "Speed hack; no|yes"                 },
        {NULL, NULL},
    };

    cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void *)vars);


    static const struct retro_controller_description port[] = {
        {"RetroPad",      RETRO_DEVICE_JOYPAD     },
        {"RetroKeyboard", RETRO_DEVICE_KEYBOARD   },
    };

    static const struct retro_controller_info ports[] = {
        {port, 2},
        {port, 2},
        {NULL, 0},
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);

} /* retro_set_environment */

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void retro_reset(void)
{
    ResetZ80(&gb);
    ResetUPD(&gb);
    ResetCRTC(&gb);

    ExecuteMenu(&gb, ID_AUTORUN, NULL); // Do: open disk & autorun // TODO
}

static int32_t WsInputGetState(core_crocods_t *core)
{
    // Hardware keyboard
    memset(gb.clav, 0xFF, 16);

    int i;
	if (core->inKeyboard == 1)
		core->inKeyboard = 0;
    for (i = 0; i < RETROK_LAST; i++) {
        int scanCode = KeySymToCPCKey[i];

        if (scanCode != CPC_NIL)
	{
            Core_Key_Sate[i] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, i);
            if (Core_Key_Sate[i] != 0)
			{
                CPC_SetScanCode(&gb, scanCode);
				core->inKeyboard = 2;
			}
        }
    }

    int32_t button = 0;

    /* Button A	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)) ? (1 << 0) : 0;
    /* Button B	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)) ? (1 << 1) : 0;

    /* SELECT BUTTON */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT)) ? (1 << 2) : 0;

    /* START BUTTON */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) ? (1 << 3) : 0;

    /* RIGHT -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) ? (1 << 4) : 0;
    /* LEFT -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) ? (1 << 5) : 0;
    /* UP -> X1		*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) ? (1 << 6) : 0;
    /* DOWN -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) ? (1 << 7) : 0;

    /* R1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R)) ? (1 << 8) : 0;
    /* L1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L)) ? (1 << 9) : 0;

    /* X    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X)) ? (1 << 10) : 0;
    /* Y    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y)) ? (1 << 11) : 0;

    /* R2    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2)) ? (1 << 14) : 0;
    /* L2    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2)) ? (1 << 15) : 0;

    return button;
}

char framebuf[128];

int frame = 0;
static int old_width;
static int old_height;

static void guestBlit(core_crocods_t *core, u16 *memBitmap, u32 width1, u32 height1, u32 left1, u32 top1, u32 bpl1, u16 *buffer_scr, u16 width2, u16 height2)
{
    u16 myScanlineMask = 0;

    if ((core->scanline > 0) && (core->scanline < 5)) {
        myScanlineMask = scanlineMask[core->scanline - 1];
    }

    u32 x, y;

    if ((old_width1 != width1) || (old_height1 != height1) || (old_left1 != left1) || (old_top1 != top1) || (old_bpl1 != bpl1) || (old_width2 != width2) || (old_height2 != height2)) {

        if ((width2 > 384 * 2) || (height2 > 272)) { // Games/demos that use splits or others video hack
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
            return;
        }

        for (x = 0; x < width2; x++) {
            incX[x] = ((x * width1) / width2) + left1;
        }
        for (y = 0; y < height2; y++) {
            incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
        }
        old_width1 = width1;
        old_height1 = height1;
        old_left1 = left1;
        old_top1 = top1;
        old_bpl1 = bpl1;
        old_width2 = width2;
        old_height2 = height2;
    }

    if (myScanlineMask != 0) {
        for (y = 0; y < height2; y++) {
            if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    u16 color = memBitmap[pos] & myScanlineMask;

                    *buffer_scr = color;
                    buffer_scr++;
                }
            } else {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        }
    } else {
        for (y = 0; y < height2; y++) {
            for (x = 0; x < width2; x++) {
                int pos = incX[x] + incY[y];

                *buffer_scr = memBitmap[pos];
                buffer_scr++;
            }
        }
    }
}


static void guestScreenDraw(core_crocods_t *core)
{
    uint16_t *buffer_scr = textureBytes;

    uint16_t *memBitmap;

    if (core->screenIsOptimized) {
        memBitmap = core->MemBitmap;
    } else {
        int ignoreRow = 0; // 3; // only in caprice32 actually
        memBitmap = core->MemBitmap + 384 * 2 * ignoreRow;
    }

    u32 width2 = 0, height2 = 0;

    if (core->resize == 2) {  // 320x200
        width2 = 320;
        height2 = 200;
    }

    if (core->resize == 1) {  // AUTO
        width2 = core->screenBufferWidth;
        height2 = core->screenBufferHeight;

        if ((width2 < 256) || (height2 < 128)) {    // Games/demos that use splits or others video hack
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
        }
    }

    if (core->resize == 4) {                // Overscan
        width2 = 384;
        height2 = 272;
    }

    if (core->lastMode == 2)
        width2 = width2 * 2;

    int x, y;

    if (core->resize == 2) {
        // ID_SCREEN_320 x 200 - keep ratio

        if (core->screenIsOptimized) {
            int dbl = 1;

            if (core->lastMode == 2)
                dbl = 2;

            // buffer_scr += 20 * actualScreen->w;      // TODO: add

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }

            #define PG_LBMASK565 0xF7DE

            #define AlphaBlendFast(pixel, backpixel) (((((pixel) & PG_LBMASK565) >> 1) | (((backpixel) & PG_LBMASK565) >> 1)))

            uint16_t *buffer_scr = textureBytes;
            char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

            uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];   // Border color

            for (y = 0; y < 20; y++) {
                for (x = 0; x < 320; x++) {
                    buffer_scr[x + y * 320] = col;
                    buffer_scr[x + (y + 220) * 320] = col;
                }
            }

            for (y = 0; y < 10; y++) {
                for (x = 0; x < pos[y]; x++) {
                    buffer_scr[x + y * 320] = 0;
                    buffer_scr[(319 - x) + y * 320] = 0;
                    buffer_scr[x + (239 - y) * 320] = 0;
                    buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                }
            }
        } else {
            u32 left1 = 32 * 2;
            u32 top1 = 40;
            u32 width1 = 320 * 2;
            u32 height1 = 200;
            u32 bpl1 = 768;     // byte per line

            // TODO: fix
            if (height2 == 240) {  // Round border on GCW
                u32 x, y;

                char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

                u16 myScanlineMask = 0;

                if ((core->scanline > 0) && (core->scanline < 5)) {
                    myScanlineMask = scanlineMask[core->scanline - 1];
                }

                uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];   // Border color

                for (y = 0; y < 20; y++) {
                    for (x = 0; x < 320; x++) {
                        if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                            u16 color = col & myScanlineMask;

                            buffer_scr[x + y * 320] = color;
                            buffer_scr[x + (y + 220) * 320] = color;
                        } else {
                            buffer_scr[x + y * 320] = col;
                            buffer_scr[x + (y + 220) * 320] = col;
                        }
                    }
                }

                for (y = 0; y < 10; y++) {
                    for (x = 0; x < pos[y]; x++) {
                        buffer_scr[x + y * 320] = 0;
                        buffer_scr[(319 - x) + y * 320] = 0;
                        buffer_scr[x + (239 - y) * 320] = 0;
                        buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                    }
                }

                height2 = 200;
                buffer_scr += 20 * width2;
            }

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);
        }
    }

    if (core->resize == 1) {   // TODO: improve resize
        // ID_SCREEN_AUTO

        if (core->screenIsOptimized) {
            int dbl = 1;

            if (core->lastMode == 2)
                dbl = 2;

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            // Copy 1 to 2

            u32 left1 = core->x0 * 2;
            u32 top1 = core->y0;
            u32 width1 = core->screenBufferWidth * 2;
            u32 height1 = core->screenBufferHeight;
            u32 bpl1 = 768;     // byte per line

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);
        }
    }

    if (core->resize == 4) {   // TODO: improve resize
        // ID_SCREEN_OVERSCAN

        if (core->screenIsOptimized) {
            int dbl = 1;

            if (core->lastMode == 2)
                dbl = 2;

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * 384 * dbl) / (width2) + ((y * 272) / (height2)) * 384 * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            u32 left1 = 0;
            u32 top1 = 0;
            u32 width1 = 768;
            u32 height1 = 272;
            u32 bpl1 = 768;     // byte per line

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);
        }
    }

    if ((core->iconTimer > 0) || (core->overlayBitmap_width != 0)) {

        int dbl_x = (core->lastMode == 2) ? 2 : 1;
        int dbl_y = 1;

        dbl_x = (width2 / 320);
        dbl_y = (height2 / 200);

        if (dbl_x < 1) {
            dbl_x = 1;
        }
        if (dbl_y < 1) {
            dbl_y = 1;
        }
        // Draw icon

        if (core->iconTimer > 0) {
            int x, y;
            int y0;

            int dispiconX = core->iconToDislay / 16;
            int dispiconY = core->iconToDislay % 16;


            uint16_t *buffer_scr = textureBytes;

            buffer_scr += +(8 * width2) + 8;

            for (y = 0; y < 32; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    int step_x = 0;
                    u16 *src = core->icons + (dispiconX * 32) + (y + dispiconY * 32) * 448;

                    for (x = 0; x < 32 * dbl_x; x++) {
                        u16 car;
                        car = *src;
                        if (car != 33840) {
                            *buffer_scr = car;
                        }
                        buffer_scr++;
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                        if (step_x == 0) {
                            src++;
                        }
                    }

                    buffer_scr += (width2 - 32 * dbl_x);
                }
            }

            // dispIcon(core, 0, 0, core->iconToDislay / 16, core->iconToDislay % 16, 0);
            core->iconTimer--;
        } // End of icon

        if (core->overlayBitmap_width != 0) {
            int y0;


            uint16_t *buffer_scr = textureBytes;

            for (y = 0; y < core->overlayBitmap_height; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    u16 *dest = buffer_scr;
                    if (core->overlayBitmap_center != 1) {
                        dest += width2 * core->overlayBitmap_posy + core->overlayBitmap_posx + (y * dbl_y + y0) * width2;
                    } else {
                        dest += width2 * ((height2 -  core->overlayBitmap_height * dbl_y) / 2)  + ((width2 -  core->overlayBitmap_width * dbl_x) / 2) + (y * dbl_y + y0) * width2;
                    }

                    // +actualScreen->w * core->overlayBitmap_posy + core->overlayBitmap_posx + y * actualScreen->w;
                    u16 *src = core->overlayBitmap + y * 320;
                    int step_x = 0;
                    for (x = 0; x < core->overlayBitmap_width * dbl_x; x++) {
                        u16 car = *src;
                        if (car != 63519) {       // RGB565(255,0,255)
                            *dest = car;
                        }
                        dest++;
                        if (step_x == 0) {
                            src++;
                        }
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                    }
                }
            }
        } // End of overlay


    }


    video_cb(textureBytes, width2, height2, width2 * 2);


} /* screen_draw */


void retro_run(void)
{
    frame++;

    static bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        updateFromEnvironment();

    input_poll_cb();

    int sndSamplerToPlay = 44100 / 50;
    int byCycle = 0;

    if (!gb.isPaused)     // Pause only the Z80
        croco_cpu_doFrame(&gb);

    u16 keys_pressed = WsInputGetState(&gb);

    if (gb.runApplication != NULL)
        gb.runApplication(&gb, (gb.wait_key_released == 1) ? 0 : keys_pressed);

    UpdateScreen(&gb);

    if (keys_pressed == 0)
        gb.wait_key_released = 0;

    if ((gb.runApplication == NULL)  && (gb.wait_key_released == 0))
        gb.ipc.keys_pressed = keys_pressed;

    nds_ReadKey(&gb);

    int width   = gb.screenBufferWidth;
    int height  = gb.screenBufferHeight;
    float ratio = (float)width / (float)height;

    if (gb.lastMode == 2)
        width = width * 2;

    if (gb.changeFilter != 0)
    {
        struct retro_game_geometry geometry;

        geometry.base_width = width;
        geometry.base_height = height;
        geometry.max_width = width;
        geometry.max_height = height;
        geometry.aspect_ratio = ratio;

        if (width != old_width || height != old_height)
                environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
        old_width = width;
        old_height = height;
        gb.changeFilter = 0;
    }

    guestScreenDraw(&gb);

    updateScreenBuffer(&gb, gb.MemBitmap, gb.screenBufferWidth);        // gb.MemBitmap not used

    gb.overlayBitmap_width = 0;

    if (sndSamplerToPlay > 0)
    {
        GB_sample_t sample[sndSamplerToPlay];
        int x;

        crocods_copy_sound_buffer(&gb, sample, sndSamplerToPlay);
        audio_batch_cb((int16_t *)&sample, sndSamplerToPlay);
    }
}

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{

}

void guestGetJoystick(core_crocods_t *core, char *string)
{

}

bool retro_load_game(const struct retro_game_info *info)
{
    struct retro_frame_time_callback frame_cb;
    struct retro_input_descriptor desc[] = {
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left"  },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up"    },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down"  },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Pause" },
        {0},
    };

    old_width = 0;
    old_height = 0;

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    // Init pixel format

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        if (log_cb)
            log_cb(RETRO_LOG_INFO, "XRGG565 is not supported.\n");
        return 0;
    }

    // Load .dsk or .sna
    strcpy(gb.openFilename, info->path);
    ExecuteMenu(&gb, ID_AUTORUN, NULL); // Do: open disk & autorun // TODO
    return 1;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_PAL;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    (void)type;
    (void)info;
    (void)num;
    return false;
}

size_t retro_serialize_size(void)
{
    return getSnapshotSize(&gb);
}

bool retro_serialize(void *data_, size_t size)
{
    int len;
    char *buffer = getSnapshot(&gb, &len);

    if (buffer != NULL) {
        if (len > size) {
            free(buffer);
            return false;
        }

        memcpy(data_, buffer, len);
        free(buffer);

        return true;
    }
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    LireSnapshotMem(&gb, (u8 *)data_);

    return true;
}

void * retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

