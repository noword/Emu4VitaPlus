#pragma once
#include <stdint.h>
#include "log.h"

namespace Emu4VitaPlus
{
    enum SOUND_ENUM
    {
        SOUND_NOTIFICATION = 0,
        SOUND_COUNT
    };

    class AudioData
    {
    public:
        AudioData(const char *wav_path);
        virtual ~AudioData();

        void Get(const uint8_t **data, size_t *count)
        {
            *data = _pcm;
            *count = _count;
        };

        bool Valid() { return _pcm != nullptr && _count > 0; };

    private:
        uint8_t *_pcm;
        size_t _count;
    };

    class Sound
    {
    public:
        Sound();
        virtual ~Sound();

        void Play(SOUND_ENUM sound);

    private:
        AudioData *_data[SOUND_COUNT];
    };
}