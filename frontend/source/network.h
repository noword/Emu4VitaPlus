#pragma once
#include <string>
#include <map>
#include <queue>
#include <psp2/net/net.h>
#include <curl/curl.h>
#include "thread_base.h"

#define DEFAULT_MAX_CONCURRENT 4

struct Response
{
    const char *data;
    size_t size;
    int code;
};

typedef void (*ClientCallBackFunc)(const Response *, void *callback_data);

enum TaskType
{
    DOWNLOAD_TASK,
    CALLBACK_TASK,
};

struct TaskBase
{
    TaskBase(TaskType t) : type(t) {};
    virtual ~TaskBase() {};
    TaskType type;
    std::string url;
    std::string post_data;
};

struct TaskDownload : public TaskBase
{
    TaskDownload() : TaskBase(DOWNLOAD_TASK), file_handle(-1) {};
    virtual ~TaskDownload() {};

    std::string file_name;
    SceUID file_handle;
};

struct TaskCallback : public TaskBase
{
    TaskCallback() : TaskBase(CALLBACK_TASK) { buf.reserve(0x200); };
    virtual ~TaskCallback() {};

    ClientCallBackFunc callback;
    void *callback_data;
    std::string buf;
};

std::string UrlEscape(std::string in);

class Network : public ThreadBase
{
public:
    Network(size_t max_concurrent = DEFAULT_MAX_CONCURRENT);
    virtual ~Network();

    bool Connected();

    size_t GetSize(const char *url);
    bool Download(const char *url, const char *file_name);
    void RestCount() { _actived_task_count = _finished_task_count = 0; };
    size_t GetFinishedCount() { return _finished_task_count; };
    size_t GetActivedCount() { return _actived_task_count; };

    // all AddTask is Non-Blocking
    void AddTask(const char *url, const char *file_name);
    void AddTask(const char *url, ClientCallBackFunc callback, void *callback_data = nullptr);
    void AddTask(const char *url, const char *post_data, size_t post_size, ClientCallBackFunc callback, void *callback_data = nullptr);

    void CleanTask();
    void CleanAllTask(); // include actived
    bool AllCompleted() { return _task_queue.empty() && _active_tasks.empty(); };

private:
    void _SetOptions(CURL *curl);
    void _ThreadLoop();
    void _SubmitTaskLoop();
    void _ActiveTaskLoop();

    static int _RunThread(SceSize args, void *argp);
    static int _init_count;

    size_t _max_concurrent;
    CURLM *_multi_handle;
    size_t _actived_task_count;
    size_t _finished_task_count;

    std::queue<TaskBase *> _task_queue;
    std::map<CURL *, TaskBase *> _active_tasks;
};