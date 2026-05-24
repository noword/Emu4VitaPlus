#include <psp2/audioout.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "sound.h"
#include "log.h"
#include "thread_base.h"

namespace Emu4VitaPlus
{

#define SOUND_OUTPUT_COUNT 256
#define SOUND_STEREO 1
#define SOUND_OUTPUT_BLOCK_SIZE (SOUND_OUTPUT_COUNT << SOUND_STEREO)
#define SOUND_SAMPLE_RATE 16000

    static const char *SoundPath[] = {
        "app0:assets/notification.wav",
    };
    struct SoundTask
    {
        const uint8_t *data;
        size_t count;
    };

#pragma pack(push, 1)

    struct WAVHeader
    {
        char riff[4];
        unsigned int file_size;
        char wave[4];
    };

    struct WAVChunkHeader
    {
        char id[4];
        unsigned int size;
    };

    struct WAVFmt
    {
        unsigned short audio_format;
        unsigned short channels;
        unsigned int sample_rate;
        unsigned int byte_rate;
        unsigned short block_align;
        unsigned short bits_per_sample;
    };

#pragma pack(pop)

    AudioData::AudioData(const char *wav_path) : _pcm(nullptr), _count(0)
    {
        LogFunctionName;
        LogDebug("  wav_path: %s", wav_path);

        FILE *fp = fopen(wav_path, "rb");
        if (!fp)
            return;

        WAVHeader header;
        WAVFmt fmt{0};

        fread(&header, sizeof(header), 1, fp);
        if (memcmp(header.riff, "RIFF", 4) != 0 ||
            memcmp(header.wave, "WAVE", 4) != 0)
        {
            goto EXIT;
        }

        while (!feof(fp))
        {
            WAVChunkHeader chunk{0};
            fread(&chunk, sizeof(chunk), 1, fp);
            if (memcmp(chunk.id, "fmt ", 4) == 0)
            {
                if (chunk.size != sizeof(WAVFmt))
                {
                    goto EXIT;
                }

                fread(&fmt, sizeof(WAVFmt), 1, fp);
                if (fmt.channels != 2)
                {
                    LogWarn("wrong channels: %d", fmt.channels);
                }

                if (fmt.sample_rate != SOUND_SAMPLE_RATE)
                {
                    LogWarn("wrong sample rate: %d", fmt.sample_rate);
                }
            }
            else if (memcmp(chunk.id, "data", 4) == 0)
            {
                if (_pcm)
                    delete[] _pcm;
                size_t bytes = ALIGN_UP(chunk.size, SOUND_OUTPUT_BLOCK_SIZE);
                _count = bytes / SOUND_OUTPUT_BLOCK_SIZE - 1;
                _pcm = new uint8_t[bytes];
                memset(_pcm, 0, bytes);
                fread(_pcm, chunk.size, 1, fp);
            }
            else
            {
                break;
            }
        }

    EXIT:
        if (fmt.sample_rate == 0 && _pcm)
        {
            delete[] _pcm;
            _pcm = nullptr;
        }

        fclose(fp);
    }

    AudioData::~AudioData()
    {
        if (_pcm)
            delete[] _pcm;
    }

    Sound::Sound()
    {
        for (int i = 0; i < SOUND_COUNT; i++)
        {
            _data[i] = new AudioData(SoundPath[i]);
        }
    }

    Sound::~Sound()
    {
        for (auto d : _data)
        {
            delete d;
        }
    }

    int PlayTask(SceSize args, void *argp)
    {
        LogFunctionName;
        SoundTask *task = (SoundTask *)argp;

        int port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, SOUND_OUTPUT_COUNT, SOUND_SAMPLE_RATE, SCE_AUDIO_OUT_MODE_STEREO);
        if (port >= 0)
        {
            static int vol[2] = {SCE_AUDIO_VOLUME_0DB / 4, SCE_AUDIO_VOLUME_0DB / 4};
            sceAudioOutSetVolume(port, (SceAudioOutChannelFlag)3, vol);

            for (int i = 0; i < task->count; i++)
            {
                sceAudioOutOutput(port, task->data);
                task->data += SOUND_OUTPUT_BLOCK_SIZE;
            }

            sceAudioOutOutput(port, NULL);
            port = sceAudioOutReleasePort(port);
            if (port < 0)
            {
                LogError("Error: sceAudioOutReleasePort() 0x%08x\n", port);
            }
        }
        else
        {
            LogError("Error: sceAudioOutOpenPort() 0x%08x\n", port);
        }

        return sceKernelExitDeleteThread(0);
    }

    void Sound::Play(SOUND_ENUM sound)
    {
        LogFunctionName;
        if (_data[sound]->Valid())
        {
            SoundTask task;
            _data[sound]->Get(&task.data, &task.count);
            LogDebug("  %08x %d", task.data, task.count);
            StartThread(PlayTask, sizeof(SoundTask), &task);
        }
    }
}