// fs_utils.c
#include "declarations.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

// 如果BUILDS_DIR没有定义，定义它
#ifndef BUILDS_DIR
#define BUILDS_DIR "builds"
#endif

// 确保builds目录存在
int ensure_builds_directory(void) {
    if (_access(BUILDS_DIR, 0) == -1) {
        // builds目录不存在，创建它
        if (_mkdir(BUILDS_DIR) == -1) {
            perror("创建builds目录失败");
            return -1;  // 返回错误代码
        }
        printf("已创建目录: %s\n", BUILDS_DIR);
    }
    return 0;  // 返回成功
}

// 同步现有目录
void sync_existing_directories_silent(void) {
    char search_path[MAX_PATH_LEN];
    struct _finddata_t fileinfo;
    intptr_t handle;

    snprintf(search_path, MAX_PATH_LEN, "%s\\*", BUILDS_DIR);

    handle = _findfirst(search_path, &fileinfo);
    if (handle == -1L) {
        return;
    }

    do {
        if (fileinfo.attrib & _A_SUBDIR) {
            // 忽略.和..
            if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0) {
                continue;
            }
            // 这里可以添加目录同步逻辑
        }
    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
}

// 创建Linux目录结构
void create_linux_directory_structure(void) {
    char real_path[MAX_PATH_LEN];

    // 确保builds目录存在
    ensure_builds_directory();

    // 创建根目录下的子目录
    const char *directories[] = {
        "bin", "dev", "etc", "home", "lib", "proc",
        "root", "sbin", "tmp", "usr", "var", NULL
    };

    for (int i = 0; directories[i] != NULL; i++) {
        snprintf(real_path, MAX_PATH_LEN, "%s/%s", BUILDS_DIR, directories[i]);
        if (_access(real_path, 0) != 0) {
            _mkdir(real_path);
        }
    }

    // 只在home目录下创建"user"文件夹
    const char *user_dir = "user";

    snprintf(real_path, MAX_PATH_LEN, "%s/home/%s", BUILDS_DIR, user_dir);
    if (_access(real_path, 0) != 0) {
        _mkdir(real_path);
    }

    // 在user目录下创建一些子目录
    const char *user_subdirs[] = {"Desktop", "Documents", "Downloads", "Music", "Pictures", "Videos", NULL};
    for (int j = 0; user_subdirs[j] != NULL; j++) {
        char subdir_path[MAX_PATH_LEN];
        snprintf(subdir_path, MAX_PATH_LEN, "%s/%s", real_path, user_subdirs[j]);
        if (_access(subdir_path, 0) != 0) {
            _mkdir(subdir_path);
        }
    }
}
