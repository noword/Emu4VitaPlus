#include <psp2/audioout.h>
#include "audio_output.h"
#include "log.h"
#include "profiler.h"

AudioOutput::AudioOutput(uint32_t sample_size, uint32_t sample_rate, AudioBuf *buf)
    : ThreadBase(_AudioThread),
      _sample_size(sample_size),
      _out_buf(buf)
{
    _port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_VOICE, sample_size, sample_rate, SCE_AUDIO_OUT_MODE_STEREO);
}

AudioOutput::~AudioOutput()
{
    sceAudioOutReleasePort(_port);
}

void AudioOutput::SetRate(uint32_t sample_size, uint32_t sample_rate)
{
    _sample_size = sample_size * 2;
    sceAudioOutSetConfig(_port, sample_size, sample_rate, SCE_AUDIO_OUT_MODE_STEREO);
}

int AudioOutput::_AudioThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(AudioOutput, output, argp);
    int16_t *buf;
    while (output->IsRunning())
    {
        while ((buf = output->_out_buf->Read(AUDIO_OUTPUT_BLOCK_SIZE)) == nullptr)
        {
            output->Wait();
            if (!output->IsRunning())
                break;
        }

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