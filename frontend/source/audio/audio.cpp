#include <speex/speex_resampler.h>
#include <psp2/audioout.h>
#include "audio.h"
#include "config.h"
#include "log.h"
#include "profiler.h"
#include "emulator.h"
#include "global.h"

#define AUDIO_SKIP_THRESHOLD 30

const uint32_t SAMPLE_RATES[] = {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

void AudioSampleCallback(int16_t left, int16_t right)
{
    LogFunctionNameLimited;
    int16_t data[2] = {left, right};
    gEmulator->_audio.SendAudioSample(data, 1);
}

size_t AudioSampleBatchCallback(const int16_t *data, size_t frames)
{
    LogFunctionNameLimited;

    if (data && frames > 0 && (!gConfig->mute) && gStatus.Get() == APP_STATUS_RUN_GAME)
    {
        BeginProfile("AudioSampleBatchCallback");
        gEmulator->_audio.SendAudioSample(data, frames);
        EndProfile("AudioSampleBatchCallback");
    }

    return frames;
}

namespace Emu4VitaPlus
{
    Audio::Audio()
        : _in_sample_rate(0),
          _resampler(nullptr),
          _output(nullptr),
          _buf_status_callback(nullptr)
    {
        LogFunctionName;

        // SetSampleRate(sample_rate);
        // LogDebug("_in_sample_rate: %d _out_sample_rate:%d _resampler:%08x", _in_sample_rate, _out_sample_rate, _resampler);
    }

    Audio::~Audio()
    {
        Deinit();
    }

    void Audio::Init(uint32_t sample_rate)
    {
        LogFunctionName;
        LogDebug("  sample_rate: %d", sample_rate);
        if (_in_sample_rate == sample_rate || sample_rate == 0)
        {
            return;
        }

        _in_sample_rate = sample_rate;

        bool need_resample = !_GetSuitableSampleRate(sample_rate, &_out_sample_rate);
        LogDebug("  need_resample: %d _out_sample_rate: %d", need_resample, _out_sample_rate);

        if (_output == nullptr)
        {
            _output = new AudioOutput(AUDIO_OUTPUT_COUNT, _out_sample_rate, &_out_buf);
            _output->Start();
        }
        else
        {
            _output->Stop();
            _output->SetRate(AUDIO_OUTPUT_COUNT, _out_sample_rate);
            _output->Start();
        }

        if (need_resample)
        {
            if (_resampler == nullptr)
            {
                _resampler = new AudioResampler(sample_rate, _out_sample_rate, _output, &_out_buf);
                _resampler->Start();
            }
            else
            {
                _resampler->Stop();
                _resampler->SetRate(sample_rate, _out_sample_rate);
                _resampler->Start();
            }
        }
    }

    void Audio::Deinit()
    {
        LogFunctionName;
        if (_resampler != nullptr)
        {
            delete _resampler;
            _resampler = nullptr;
        }

        if (_output != nullptr)
        {
            delete _output;
            _output = nullptr;
        }
    }

    bool Audio::_GetSuitableSampleRate(uint32_t sample_rate, uint32_t *out_sample_rate)
    {
        LogFunctionName;

        for (auto const &psv_sample_rate : SAMPLE_RATES)
        {
            if ((sample_rate >= psv_sample_rate - SAMPLE_RATE_NEGLECT) && (sample_rate <= psv_sample_rate + SAMPLE_RATE_NEGLECT))
            {
                *out_sample_rate = psv_sample_rate;
                return true;
            }
            else if (psv_sample_rate < sample_rate)
            {
                *out_sample_rate = psv_sample_rate;
            }
        }

        return false;
    }
    // #define SAVE_AUDIO
    size_t Audio::SendAudioSample(const int16_t *data, size_t frames)
    {
#ifdef SAVE_AUDIO
        static FILE *fp = nullptr;
        if (fp == nullptr)
        {
            fp = fopen(ROOT_DIR "/audio.bin", "wb");
        }
        fwrite(data, frames * 2, 1, fp);
#endif
        if (_resampler != nullptr)
        {
            _resampler->Process(data, frames);
        }
        else
        {
            _out_buf.Write(data, frames * 2);
            _output->Signal();
        }

        // LogDebug("%d %d", frames * 2, _out_buf.OccupancySize());

        return frames;
    }

    void Audio::NotifyBufStatus()
    {
        if (_buf_status_callback && _output)
        {
            unsigned occupancy = _output->GetOccupancy();
            _buf_status_callback(gConfig->mute, occupancy, occupancy < AUDIO_SKIP_THRESHOLD);
            // if (occupancy < AUDIO_SKIP_THRESHOLD)
            // {
            //     LogDebug("skip audio: %d", occupancy);
            // }
        }
    }
}