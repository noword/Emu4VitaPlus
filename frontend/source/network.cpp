#include <psp2/net/http.h>
#include <psp2/sysmodule.h>
#include <psp2/net/netctl.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include "network.h"
#include "global.h"
#include "file.h"
#include "log.h"

#define USER_AGENT "Emu4Vita++/" APP_VER_STR
#define POOL_SIZE (1 * 1024 * 1024)

int Network::_init_count = 0;

std::string UrlEscape(std::string in)
{
    std::string out;

    CURL *curl;
    if ((!in.empty()) && (curl = curl_easy_init()) != NULL)
    {
        for (size_t i = 0; i < in.size(); i++)
        {
            if (in[i] == '&')
                in[i] = '_';
        }

        char *esc = curl_easy_escape(curl, in.c_str(), in.size());
        if (esc)
        {
            out = esc;
            curl_free(esc);
        }

        curl_easy_cleanup(curl);
    }

    return out;
}

static size_t FileWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    return sceIoWrite(*(SceUID *)userdata, ptr, size * nmemb);
}

Network::Network(size_t max_concurrent)
    : ThreadBase(_RunThread),
      _max_concurrent(max_concurrent),
      _finished_task_count(0)
{
    LogFunctionName;
    if (_init_count == 0)
    {
        sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
        sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

        SceNetInitParam init_param{new uint8_t[POOL_SIZE], POOL_SIZE, 0};
        sceNetInit(&init_param);
        sceNetCtlInit();

        sceHttpInit(POOL_SIZE);
        int res = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (res != CURLE_OK)
        {
            LogError("curl_global_init: %08x", res);
        }
    }

    _init_count++;

    _multi_handle = curl_multi_init();
    if (!_multi_handle)
    {
        LogWarn("Failed to create CURLM handle");
    }
}

Network::~Network()
{
    LogFunctionName;

    Stop();

    while (!_task_queue.empty())
    {
        delete _task_queue.front();
        _task_queue.pop();
    }

    for (auto task : _active_tasks)
    {
        delete task.second;
    }

    curl_multi_cleanup(_multi_handle);

    _init_count--;
    if (_init_count == 0)
    {
        curl_global_cleanup();
        sceHttpTerm();

        sceNetCtlTerm();
        sceNetTerm();

        sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
    }
}

bool Network::Connected()
{
    int state;
    sceNetCtlInetGetState(&state);
    return state == SCE_NETCTL_STATE_CONNECTED;
}

int Network::_RunThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(Network, network, argp);

    network->_ThreadLoop();

    sceKernelExitThread(0);
    return 0;
}

void Network::_ThreadLoop()
{
    while (IsRunning() && (gStatus.Get() & (APP_STATUS_EXIT | APP_STATUS_RETURN_ARCH | APP_STATUS_REBOOT_WITH_LOADING)) == 0)
    {
        if (_task_queue.empty() && _active_tasks.empty())
        {
            uint32_t timeout = 10000;
            if (Wait(&timeout) == SCE_KERNEL_ERROR_WAIT_TIMEOUT)
            {
                continue;
            }
        }

        while (!_task_queue.empty() && _active_tasks.size() < _max_concurrent)
        {
            auto task = _task_queue.front();
            _task_queue.pop();

            CURL *easy = curl_easy_init();
            if (!easy)
            {
                LogWarn("Failed to create CURL easy handle");
                goto TASK_ERROR;
            }

            _SetOptions(easy);
            curl_easy_setopt(easy, CURLOPT_URL, task->url.c_str());

            switch (task->type)
            {
            case DOWNLOAD_TASK:
            {
                TaskDownload *download_task = (TaskDownload *)task;
                download_task->file_handle = sceIoOpen(download_task->file_name.c_str(), SCE_O_WRONLY | SCE_O_CREAT, 0777);
                if (download_task->file_handle < 0)
                {
                    LogWarn("Failed to open file: %s error: %08x", download_task->file_name.c_str(), download_task->file_handle);
                    goto TASK_ERROR;
                }

                curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, FileWriteCallback);
                curl_easy_setopt(easy, CURLOPT_WRITEDATA, &download_task->file_handle);
            }
            break;

            case CALLBACK_TASK:
                break;

            default:
                LogWarn("unknown task type: %d", task->type);
                goto TASK_ERROR;
                break;
            }

            curl_multi_add_handle(_multi_handle, easy);
            _active_tasks[easy] = task;
            continue;

        TASK_ERROR:
            if (easy)
            {
                curl_easy_cleanup(easy);
            }

            delete task;
        }

        int still_running = 0;
        curl_multi_perform(_multi_handle, &still_running);

        int numfds;
        curl_multi_wait(_multi_handle, NULL, 0, 50, &numfds);

        CURLMsg *msg;
        int msgs_left;
        while ((msg = curl_multi_info_read(_multi_handle, &msgs_left)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                CURL *easy = msg->easy_handle;
                auto it = _active_tasks.find(easy);
                if (it == _active_tasks.end())
                {
                    LogWarn("incorrect easy handle: %08x", easy);
                    continue;
                }

                auto *task = it->second;
                switch (task->type)
                {
                case DOWNLOAD_TASK:
                {
                    TaskDownload *download_task = (TaskDownload *)task;
                    if (download_task->file_handle > 0)
                    {
                        sceIoClose(download_task->file_handle);
                    }

                    if (msg->data.result == CURLE_OK)
                    {
                        LogDebug("  download: %s", download_task->file_name.c_str());
                        _finished_task_count++;
                    }
                    else
                    {
                        LogWarn("  download failed: %s", download_task->file_name.c_str());
                        LogWarn("  result: %d", msg->data.result);
                        File::Remove(download_task->file_name.c_str());
                    }
                }
                break;

                case CALLBACK_TASK:
                    break;

                default:
                    LogWarn("unknown active task type: %d", task->type);
                    break;
                }

                _active_tasks.erase(it);
                curl_multi_remove_handle(_multi_handle, easy);
                curl_easy_cleanup(easy);
            }
        }
    }
}

void Network::AddTask(const char *url, const char *file_name)
{
    LogFunctionName;

    TaskDownload *task = new TaskDownload;
    task->url = url;
    task->file_name = file_name;

    Lock();
    _task_queue.push(task);
    Unlock();

    Signal();
}

void Network::AddTask(const char *url, const char *post_data, ClientCallBackFunc callback)
{
    LogFunctionName;

    TaskCallback *task = new TaskCallback;
    task->callback = callback;
    task->post_data = post_data;

    Lock();
    _task_queue.push(task);
    Unlock();

    Signal();
}

void Network::_SetOptions(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
}

size_t Network::GetSize(const char *url)
{
    LogFunctionName;
    LogDebug("  url: %s", url);

    if (!Connected())
        return 0;

    CURL *curl = curl_easy_init();
    if (!curl)
        return 0;

    double content_length = 0.;

    _SetOptions(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LogWarn("  curl_easy_perform error:%d", res);
        goto END;
    }

    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);

    if (res != CURLE_OK)
    {
        LogWarn("  curl_easy_getinfo error:%d", res);
        goto END;
    }

END:
    curl_easy_cleanup(curl);

    return (size_t)content_length;
}

bool Network::Download(const char *url, const char *file_name)
{
    LogFunctionName;
    LogDebug("  url: %s", url);
    LogDebug("  file_name: %s", file_name);

    if (!Connected())
        return false;

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        LogDebug("curl_easy_init failed");
        return false;
    }

    SceUID fp = sceIoOpen(file_name, SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (fp < 0)
    {
        LogError("failed to create file: %s", file_name);
        curl_easy_cleanup(curl);
        return false;
    }

    _SetOptions(curl);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fp);
    CURLcode res = curl_easy_perform(curl);

    sceIoClose(fp);

    curl_easy_cleanup(curl);

    if (res == CURLE_OK)
    {
        return true;
    }
    else
    {
        LogDebug("download failed");
        File::Remove(file_name);
        return false;
    }
}