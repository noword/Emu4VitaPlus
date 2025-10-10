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
    TaskDownload() : TaskBase(DOWNLOAD_TASK) {};
    virtual ~TaskDownload() {};

    std::string file_name;
    SceUID file_handle = -1;
};

struct TaskCallback : public TaskBase
{
    TaskCallback() : TaskBase(CALLBACK_TASK) {};
    virtual ~TaskCallback() {};

    ClientCallBackFunc callback = nullptr;
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
    void AddTask(const char *url, const char *file_name);
    void AddTask(const char *url, const char *post_data, ClientCallBackFunc callback);
    size_t GetFinishedCount() { return _finished_task_count; };

private:
    void _SetOptions(CURL *curl);
    void _ThreadLoop();

    static int _RunThread(SceSize args, void *argp);
    static int _init_count;

    size_t _max_concurrent;
    CURLM *_multi_handle;
    size_t _finished_task_count;

    std::queue<TaskBase *> _task_queue;
    std::map<CURL *, TaskBase *> _active_tasks;
};