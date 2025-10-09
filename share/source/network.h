#pragma once
#include <string>
#include <queue>
#include <functional>
#include <map>
#include <stdint.h>
#include <psp2/net/net.h>
#include <curl/curl.h>

namespace Network
{
    using FetchCallbackFunc = std::function<void(uint8_t *data, uint64_t size)>;

    void Init();
    void Deinit();

    bool Connected();
    std::string Escape(std::string in);

    void Fetch(const char *url, FetchCallbackFunc callback);
    void Fetch(const char *url, char *post_data, size_t post_size, FetchCallbackFunc callback);
    bool Fetch(const char *url, char *post_data, size_t post_size, uint8_t **data, uint64_t *size); // must delete the data pointer, if return value is true
    bool Fetch(const char *url, uint8_t **data, uint64_t *size);                                    // must delete the data pointer, if return value is true
    bool Download(const char *url, const char *dest_path);
    size_t GetSize(const char *url);

    class MultiDownloader
    {
    public:
        MultiDownloader(size_t max_concurrent = 4);
        ~MultiDownloader();

        bool Inited() { return _multi_handle != NULL; };
        void SetMaxConcurrent(size_t n);
        void AddTask(const std::string &url, const std::string &file_name);
        int ClearTask();
        int Perform();
        bool AllCompleted() const;

    private:
        struct TaskInfo
        {
            std::string url;
            std::string file_name;
        };

        struct DownloadTask
        {
            std::string file_name;
            SceUID file;
            CURL *easy_handle;
        };

        CURLM *_multi_handle;
        size_t _max_concurrent;

        std::queue<TaskInfo> _pending_tasks;
        std::map<CURL *, DownloadTask> _active_tasks;
    };

};
