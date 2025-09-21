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

namespace Network
{
    static bool NetworkInited = false;

    void Init()
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

        NetworkInited = true;
    }

    void Deinit()
    {
        sceHttpTerm();

        sceNetCtlTerm();
        sceNetTerm();

        sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        curl_global_cleanup();
        NetworkInited = false;
    }

    bool Connected()
    {
        if (NetworkInited)
        {
            int state;
            sceNetCtlInetGetState(&state);
            return state == SCE_NETCTL_STATE_CONNECTED;
        }
        else
        {
            return false;
        }
    }

    std::string Escape(std::string in)
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

    static inline void SetOptions(CURL *curl)
    {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    }

    struct DownloadBuffer
    {
        uint8_t *data;
        uint64_t size;
    };

    static size_t MemroyWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        size_t total_size = size * nmemb;
        DownloadBuffer *buf = (DownloadBuffer *)userdata;
        memcpy(buf->data + buf->size, ptr, total_size);
        buf->size += total_size;
        return total_size;
    }

    static size_t FileWriteCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
    {
        return sceIoWrite(*(SceUID *)userdata, ptr, size * nmemb);
    }

    bool Download(const char *url, uint8_t **data, uint64_t *size)
    {
        LogFunctionName;
        LogDebug("  url: %s", url);

        if (!Connected())
            return false;

        CURL *curl = curl_easy_init();
        if (!curl)
            return false;

        bool result = false;
        double content_length = -1.0;
        DownloadBuffer dl{nullptr, 0};

        *data = nullptr;
        *size = 0;

        SetOptions(curl);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            LogWarn("  curl_easy_perform error:%d", res);
            goto END;
        }

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);

        if (res != CURLE_OK || content_length <= 0)
        {
            LogWarn("  curl_easy_getinfo error:%d", res);
            goto END;
        }

        *size = content_length;

        LogDebug("  size: %f %d", content_length, *size);

        dl.data = *data = new uint8_t[*size + 1];

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MemroyWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dl);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK || dl.size != *size)
        {
            LogWarn("  2nd curl_easy_perform error: %d", res);
            delete[] data;
            goto END;
        }

        (*data)[*size] = '\x00';
        result = true;
    END:
        curl_easy_cleanup(curl);
        return result;
    }

    bool Download(const char *url, const char *dest_path)
    {
        LogFunctionName;
        LogDebug("  url: %s", url);
        LogDebug("  urdest_pathl: %s", dest_path);

        if (!Connected())
            return 0;

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            LogDebug("curl_easy_init failed");
            return false;
        }

        SceUID fp = sceIoOpen(dest_path, SCE_O_WRONLY | SCE_O_CREAT, 0777);
        if (fp < 0)
        {
            LogError("failed to create file: %s", dest_path);
            return false;
        }

        SetOptions(curl);

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
            File::Remove(dest_path);
            return false;
        }
    }

    size_t GetSize(const char *url)
    {
        LogFunctionName;
        LogDebug("  url: %s", url);

        if (!Connected())
            return 0;

        CURL *curl = curl_easy_init();
        if (!curl)
            return 0;

        double content_length = 0.;

        SetOptions(curl);
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

    MultiDownloader::MultiDownloader(size_t max_concurrent) : _max_concurrent(max_concurrent)
    {
        _multi_handle = curl_multi_init();
        if (!_multi_handle)
        {
            LogWarn("Failed to create CURLM handle");
        }
    }

    MultiDownloader::~MultiDownloader()
    {
        for (auto &pair : _active_tasks)
        {
            if (pair.second.easy_handle)
            {
                curl_multi_remove_handle(_multi_handle, pair.second.easy_handle);
                curl_easy_cleanup(pair.second.easy_handle);
            }
            if (pair.second.file)
            {
                sceIoClose(pair.second.file);
            }
        }
        curl_multi_cleanup(_multi_handle);
    }

    void MultiDownloader::SetMaxConcurrent(size_t n)
    {
        _max_concurrent = n;
    }

    void MultiDownloader::AddTask(const std::string &url, const std::string &file_name)
    {
        LogFunctionName;
        LogDebug("  ulr: %s file_name: %s", url.c_str(), file_name.c_str());
        _pending_tasks.push({url, file_name});
    }

    int MultiDownloader::ClearTask()
    {
        LogFunctionName;
        int count = 0;
        while (!_pending_tasks.empty())
        {
            count++;
            _pending_tasks.pop();
        }
        return count;
    }

    int MultiDownloader::Perform()
    {
        while (!_pending_tasks.empty() && _active_tasks.size() < _max_concurrent)
        {
            auto task = _pending_tasks.front();
            _pending_tasks.pop();

            CURL *easy = curl_easy_init();
            if (!easy)
            {
                LogWarn("Failed to create CURL easy handle");
                break;
            }

            SceUID fp = sceIoOpen(task.file_name.c_str(), SCE_O_WRONLY | SCE_O_CREAT, 0777);
            if (fp < 0)
            {
                curl_easy_cleanup(easy);
                LogWarn("Failed to open file: %s", task.file_name.c_str());
            }

            LogDebug("  open: %08x", fp);
            DownloadTask active_task{task.file_name, fp, easy};
            _active_tasks[easy] = active_task;

            SetOptions(easy);
            curl_easy_setopt(easy, CURLOPT_URL, task.url.c_str());
            curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, FileWriteCallback);
            curl_easy_setopt(easy, CURLOPT_WRITEDATA, &(_active_tasks[easy].file));

            curl_multi_add_handle(_multi_handle, easy);
        }

        int still_running = 0;
        curl_multi_perform(_multi_handle, &still_running);

        int numfds;
        curl_multi_wait(_multi_handle, NULL, 0, 50, &numfds);

        CURLMsg *msg;
        int msgs_left;
        int done = 0;
        while ((msg = curl_multi_info_read(_multi_handle, &msgs_left)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                CURL *easy = msg->easy_handle;
                auto it = _active_tasks.find(easy);
                if (it != _active_tasks.end())
                {
                    if (it->second.file >= 0)
                    {
                        sceIoClose(it->second.file);
                    }

                    if (msg->data.result == CURLE_OK)
                    {
                        LogDebug("  download: %s", it->second.file_name.c_str());
                        done++;
                    }
                    else
                    {
                        LogWarn("  download failed: %s", it->second.file_name.c_str());
                        LogWarn("  result: %d", msg->data.result);
                        File::Remove(it->second.file_name.c_str());
                    }

                    _active_tasks.erase(it);
                    curl_multi_remove_handle(_multi_handle, easy);
                    curl_easy_cleanup(easy);
                }
            }
        }
        return done;
    }

    bool MultiDownloader::AllCompleted() const
    {
        return _pending_tasks.empty() && _active_tasks.empty();
    }

}
