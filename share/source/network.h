#pragma once
#include <stdint.h>
#include <psp2/net/net.h>

class Network
{
public:
    Network(int size = 1024 * 1024);
    virtual ~Network();

    // must delete the return pointer, if it's not nullpr
    uint8_t *Download(const char *url, uint64_t *size);

private:
    SceNetInitParam _init_param;
    int _template_id;
};