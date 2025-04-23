#pragma once
#include <utility>
#include "thread_base.h"
#include "audio_define.h"
#include "audio_buf.h"

class AudioOutput : public ThreadBase
{
public:
    AudioOutput(uint32_t sample_size, uint32_t sample_rate, AudioBuf *buf);
    virtual ~AudioOutput();
    void SetRate(uint32_t sample_size, uint32_t sample_rate);
    unsigned GetOccupancy();

private:
    static int
    _AudioThread(SceSize args, void *argp);

    int _port;
    uint32_t _sample_size;
    AudioBuf *_out_buf;
};