#pragma once
#include <string>
#include <stdint.h>
#include <psp2/net/net.h>

class Network
{
public:
    Network(int size = 1024 * 1024);
    virtual ~Network();

    // must delete the data pointer, if return value is true
    bool Download(const char *url, uint8_t **data, uint64_t *size);
    bool Download(const char *url, const char *dest_path);
    std::string Escape(std::string in);

private:
    static SceNetInitParam _init_param;
    static int _template_id;
    static int _count;
};