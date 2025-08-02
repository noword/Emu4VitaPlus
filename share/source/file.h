#pragma once
#include <psp2/io/stat.h>
#include <string>

#define SCE_FIOS_PATH_MAX 1024

namespace File
{
    bool Exist(const char *path);
    bool GetCreateTime(const char *name, SceDateTime *time);
    bool GetCreateTime(const char *name, time_t *time);
    size_t GetSize(const char *name);
    bool ReadFile(const char *name, void *buf, SceSSize size);
    size_t ReadFile(const char *name, void **buf);
    size_t ReadCompressedFile(const char *name, void **buf);
    bool WriteFile(const char *name, void *buf, SceSSize size);
    bool CopyFile(const char *src_path, const char *dst_path);
    bool MoveFile(const char *src_path, const char *dst_path);
    void MakeDirs(const char *path, SceIoMode mode = (SceIoMode)0777);
    bool Remove(const char *path);
    void RemoveAllFiles(const char *path);
    std::string GetName(const char *path);
    std::string GetStem(const char *path);
    std::string GetExt(const char *path, bool lower = true);
    std::string GetDir(const char *path);
    uint32_t GetCrc32(const char *name);
    size_t GetFileCount(const char *path);
}