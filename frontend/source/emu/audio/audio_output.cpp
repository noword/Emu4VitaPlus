#include <psp2/audioout.h>
#include "audio_output.h"
#include "log.h"
#include "profiler.h"

AudioOutput::AudioOutput(uint32_t sample_size, uint32_t sample_rate, AudioBuf *buf)
    : ThreadBase(_AudioThread, "audio"),
      _sample_size(sample_size),
      _out_buf(buf),
      _latency_size(0),
      _latency_updated(false)
{
    _port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_VOICE, sample_size, sample_rate, SCE_AUDIO_OUT_MODE_STEREO);
}

AudioOutput::~AudioOutput()
{
    LogFunctionName;
    _keep_running = false;
    sceAudioOutReleasePort(_port);
}

void AudioOutput::SetRate(uint32_t sample_size, uint32_t sample_rate)
{
    _sample_size = sample_size * 2;
    sceAudioOutSetConfig(_port, sample_size, sample_rate, SCE_AUDIO_OUT_MODE_STEREO);
    _latency_updated = true;
}

int AudioOutput::_AudioThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(AudioOutput, output, argp);
    int16_t *buf;
    while (output->IsRunning())
    {
        if (unlikely(output->_latency_updated))
        {
            while (output->_out_buf->AvailableSize() < output->_latency_size && output->IsRunning())
            {
                output->Wait();
            }
            output->_latency_updated = false;
        }

        while ((buf = output->_out_buf->Read(AUDIO_OUTPUT_BLOCK_SIZE)) == nullptr && output->IsRunning())
        {
            output->Wait();
        }

        if (unlikely(!output->IsRunning()))
            break;

        BeginProfile("AudioOutput");
        sceAudioOutOutput(output->_port, buf);
        EndProfile("AudioOutput");
    }

    LogDebug("_AudioThread exit");
    sceKernelExitThread(0);

    return 0;
}

unsigned AudioOutput::GetOccupancy()
{
    int rest = sceAudioOutGetRestSample(_port);
    if (rest >= 0)
    {
        return rest * 100 / _sample_size;
    }

    return 0;
}

int AudioOutput::GetRemain()
{
    int rest = sceAudioOutGetRestSample(_port);
    return rest > 0 ? rest : 0;
}
