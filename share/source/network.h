#pragma once
#include <string>
#include <stdint.h>
#include <psp2/net/net.h>
#include <curl/curl.h>

class Network
{
public:
public:
    static Network *GetInstance()
    {
        if (_instance == nullptr)
            _instance = new Network;
        return _instance;
    }

    static void Clean()
    {
        if (_instance)
        {
            delete _instance;
            _instance = nullptr;
        }
    }

    // must delete the data pointer, if return value is true
    bool Download(const char *url, uint8_t **data, uint64_t *size);
    bool Download(const char *url, const char *dest_path);
    std::string Escape(std::string in);

private:
    Network();
    virtual ~Network();

    Network(Network const &) = delete;
    void operator=(Network const &) = delete;

    static size_t _WriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata);

    static Network *_instance;
    static CURL *_curl;
};