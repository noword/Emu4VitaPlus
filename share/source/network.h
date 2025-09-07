#pragma once
#include <string>
#include <queue>
#include <unordered_map>
#include <stdint.h>
#include <psp2/net/net.h>
#include <curl/curl.h>

namespace Network
{

    void Init();
    void Deinit();

    bool Connected();
    std::string Escape(std::string in);

    bool Download(const char *url, uint8_t **data, uint64_t *size); // must delete the data pointer, if return value is true
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
        std::unordered_map<CURL *, DownloadTask> _active_tasks;
    };

};
