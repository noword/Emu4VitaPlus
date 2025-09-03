#include <psp2/net/http.h>
#include <psp2/sysmodule.h>
#include <psp2/net/netctl.h>
#include <psp2/libssl.h>
#include "network.h"
#include "file.h"
#include "log.h"

// #define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36"
#define USER_AGENT "libhttp/3.65 (PS Vita)"

SceNetInitParam Network::_init_param;
int Network::_template_id;
int Network::_count = 0;

Network::Network(int size)
{
    LogFunctionName;
    if (_count == 0)
    {
        _init_param = {new uint8_t[size], size, 0};

        sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
        sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
        sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);

        sceNetInit(&_init_param);
        sceNetCtlInit();
        sceSslInit(size);
        sceHttpInit(size);
        sceHttpsDisableOption(SCE_HTTPS_FLAG_SERVER_VERIFY);

        if ((_template_id = sceHttpCreateTemplate(USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE)) < 0)
        {
            LogError("sceHttpCreateTemplate failed: %08x", _template_id);
        }
    }
    _count++;
}

Network::~Network()
{
    LogFunctionName;

    _count--;
    if (_count == 0)
    {
        if (_template_id > 0)
            sceHttpDeleteTemplate(_template_id);

        sceHttpTerm();
        sceSslTerm();
        sceNetCtlTerm();
        sceNetTerm();

        sceSysmoduleUnloadModule(SCE_SYSMODULE_SSL);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        // delete[] (uint8_t *)_init_param.memory;
    }
}

bool Network::Download(const char *url, uint8_t **data, uint64_t *size)
{
    LogFunctionName;
    LogDebug("  url: %s", url);
    int connection_id = 0;
    int request_id = 0;
    int ret, status_code;

    *data = nullptr;
    *size = 0;

    if ((connection_id = sceHttpCreateConnectionWithURL(_template_id, url, SCE_TRUE)) < 0)
    {
        LogWarn("sceHttpCreateConnectionWithURL failed: %08x", connection_id);
        goto END;
    }

    if ((request_id = sceHttpCreateRequestWithURL(connection_id, SCE_HTTP_METHOD_GET, url, 0)) < 0)
    {
        LogWarn("sceHttpCreateRequestWithURL failed: %08x", request_id);
        goto END;
    }

    if ((ret = sceHttpSendRequest(request_id, NULL, 0)) < 0)
    {
        LogWarn("sceHttpSendRequest failed: %08x", ret);
        goto END;
    }

    if (((ret = sceHttpGetStatusCode(request_id, &status_code)) < 0) || (status_code != 200))
    {
        LogWarn("sceHttpGetStatusCode failed: %08x %d", ret, status_code);
        goto END;
    }

    if ((ret = sceHttpGetResponseContentLength(request_id, size)) < 0)
    {
        LogWarn("sceHttpGetResponseContentLength failed: %08x", ret);
        goto END;
    }

    *data = new uint8_t[*size + 1];
    if ((ret = sceHttpReadData(request_id, *data, *size + 1)) != *size)
    {
        LogWarn("sceHttpReadData failed: %08x", ret);
        delete[] *data;
        *data = nullptr;
        *size = 0;
        goto END;
    }

    (*data)[*size] = '\x00';

END:
    if (request_id > 0)
        sceHttpDeleteRequest(request_id);

    if (connection_id > 0)
        sceHttpDeleteConnection(connection_id);

    return *data != nullptr;
}

bool Network::Download(const char *url, const char *dest_path)
{
    LogFunctionName;

    bool result = false;
    uint8_t *data;
    uint64_t size;
    if (Download(url, &data, &size))
    {
        result = File::WriteFile(dest_path, data, size);
        delete[] data;
    }
    return result;
}

std::string Network::Escape(std::string in)
{
    int ret;
    size_t esc_size;

    for (size_t i = 0; i < in.size(); i++)
    {
        if (in[i] == '&')
            in[i] = '_';
    }

    if ((ret = sceHttpUriEscape(NULL, &esc_size, 0, in.c_str())) < 0)
    {
        printf("sceHttpUriEscape() returns %x.\n", ret);
        return "";
    }

    char *esc = new char[esc_size];
    if ((ret = sceHttpUriEscape(esc, &esc_size, esc_size, in.c_str())) < 0)
    {
        printf("sceHttpUriEscape() returns %x.\n", ret);
    }

    std::string out{esc};
    delete[] esc;
    return out;
}