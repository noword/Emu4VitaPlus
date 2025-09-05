#include <psp2/net/http.h>
#include <psp2/sysmodule.h>
#include <psp2/net/netctl.h>
#include <psp2/io/fcntl.h>
#include <string.h>
#include "network.h"
#include "file.h"
#include "log.h"

// #define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36"
#define USER_AGENT "libhttp/3.65 (PS Vita)"
#define POOL_SIZE (1 * 1024 * 1024)

Network *Network::_instance = nullptr;

struct DownloadBuffer
{
    uint8_t *data;
    uint64_t size;
};

Network::Network() : _curl(nullptr),
                     _connected(false)
{
    LogFunctionName;

    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

    SceNetInitParam init_param{new uint8_t[POOL_SIZE], POOL_SIZE, 0};
    sceNetInit(&init_param);
    sceNetCtlInit();

    sceHttpInit(POOL_SIZE);

    int state;
    sceNetCtlInetGetState(&state);
    LogDebug("  sceNetCtlInetGetState: %d", state);
    _connected = (state == SCE_NETCTL_STATE_CONNECTED);

    curl_global_init(CURL_GLOBAL_ALL);
    _curl = curl_easy_init();
    if (_curl)
    {
        curl_easy_setopt(_curl, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        // curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 1L);
    }
}

Network::~Network()
{
    LogFunctionName;
    if (_curl)
        curl_easy_cleanup(_curl);
    curl_global_cleanup();

    sceHttpTerm();

    sceNetCtlTerm();
    sceNetTerm();

    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

bool Network::Download(const char *url, uint8_t **data, uint64_t *size)
{
    LogFunctionName;
    LogDebug("  url: %s", url);
    if (!_curl)
        return false;

    *data = nullptr;
    *size = 0;

    curl_easy_setopt(_curl, CURLOPT_URL, url);
    curl_easy_setopt(_curl, CURLOPT_NOBODY, 1);
    CURLcode res = curl_easy_perform(_curl);

    if (res != CURLE_OK)
    {
        LogWarn("  curl_easy_perform error:%d", res);
        return false;
    }

    double content_length = -1.0;
    res = curl_easy_getinfo(_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);

    if (res != CURLE_OK || content_length <= 0)
    {
        LogWarn("  curl_easy_getinfo error:%d", res);
        return false;
    }

    *size = content_length;
    LogDebug("  size: %f %d", content_length, *size);
    *data = new uint8_t[*size + 1];

    DownloadBuffer dl{*data, 0};

    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _MemroyWriteCallback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &dl);
    curl_easy_setopt(_curl, CURLOPT_NOBODY, 0);

    res = curl_easy_perform(_curl);

    if (res != CURLE_OK || dl.size != *size)
    {
        LogWarn("  2nd curl_easy_perform error: %d", res);
        delete[] data;
        return false;
    }

    (*data)[*size] = '\x00';

    return true;
}

bool Network::Download(const char *url, const char *dest_path)
{
    LogFunctionName;

    SceUID fp = sceIoOpen(dest_path, SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (!fp)
    {
        LogError("failed to create file: %s", dest_path);
        return false;
    }

    curl_easy_setopt(_curl, CURLOPT_URL, url);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, _FileWriteCallback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &fp);
    CURLcode res = curl_easy_perform(_curl);

    sceIoClose(fp);

    if (res == CURLE_OK)
    {
        return true;
    }
    else
    {
        File::Remove(dest_path);
        return false;
    }
}

std::string Network::Escape(std::string in)
{
    std::string out;
    if (_curl)
    {
        for (size_t i = 0; i < in.size(); i++)
        {
            if (in[i] == '&')
                in[i] = '_';
        }

        char *esc = curl_easy_escape(_curl, in.c_str(), in.size());
        if (esc)
        {
            out = esc;
            curl_free(esc);
        }
    }

    return out;
}

size_t Network::_MemroyWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total_size = size * nmemb;
    DownloadBuffer *buf = (DownloadBuffer *)userdata;
    memcpy(buf->data + buf->size, ptr, total_size);
    buf->size += total_size;
    return total_size;
}

size_t Network::_FileWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    return sceIoWrite(*(SceUID *)userdata, ptr, size * nmemb);
}