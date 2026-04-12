// declarations.h
#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 版本
#define LGD_OS_VERSION "LGD-OS 1.1 Pre-2"
#define BUILDS_DIR "builds"

// 常量定义
#define MAX_CMD_LEN 1024
#define MAX_PATH_LEN 1024
#define MAX_ARGS 32
#define MAX_HISTORY 100
#define MAX_FILENAME 256
#define MAX_CONTENT_LEN 4096
#define MAX_PIPES 10
#define MAX_INPUT_LEN 1024
#define MAX_TOKENS 100
#define MAX_OUTPUT_LEN 8192

// 包含log.h
#include "log.h"

// Shell上下文结构体
typedef struct {
    char current_dir[MAX_PATH_LEN];
    char username[50];
    char hostname[50];
    char home_dir[MAX_PATH_LEN];
    int running;
    int history_count;
    char *history[MAX_HISTORY];  // 历史记录
    Logger *logger;              // 日志器
} ShellContext;

// 命令结构体
typedef struct {
    char *name;                                   // 命令名
    int (*func)(ShellContext *, int, char *[], FILE *, FILE *);  // 函数指针
    char *brief;                                  // 简要说明
    char *detail;                                 // 详细说明
} Command;

// 全局命令数组和数量
extern Command commands[];
extern int command_count;

// Shell初始化、清理、运行
ShellContext *shell_init(void);
void shell_cleanup(ShellContext *ctx);
void shell_run(ShellContext *ctx);
void shell_display_prompt(ShellContext *ctx);
int shell_execute_command(ShellContext *ctx, const char *cmd_line, FILE *in, FILE *out);
int shell_execute_pipeline(ShellContext *ctx, const char *pipeline_cmd);

// 控制台颜色
void set_console_color(int color);
void reset_console_color(void);
void get_hostname(char *buffer, int size);
void get_username(char *buffer, int size);

// 文件系统辅助函数
int ensure_builds_directory(void);  // 返回int类型
void create_linux_directory_structure(void);
int create_directory_if_not_exists(const char *path);
int create_directory_if_not_exists_silent(const char *path);
void real_to_virtual_path(const char *real_path, char *virtual_path);
void sync_existing_directories_silent(void);
int handle_unknown_command(ShellContext *ctx, const char *cmd, int argc, char *argv[], FILE *in, FILE *out);

// 路径处理
void normalize_path(char *path);
void virtual_to_real_path(const char *virtual_path, char *real_path);
int is_directory(const char *path);
int is_file(const char *path);
void remove_quotes(char *str);
int parse_input(char *input, int *argc, char *argv[]);
void display_prompt(ShellContext *ctx);
int execute_command(ShellContext *ctx, const char *input, FILE *in, FILE *out);

// 添加缺少的函数声明
void get_file_info_string(const char *path, char *info_str, int buffer_size);

// 命令函数声明
int cmd_cd(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_ls(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_ll(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_pwd(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_mkdir(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_rmdir(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_cp(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_mv(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_touch(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_rm(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_cat(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_edit(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_info(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_echo(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_grep(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_wc(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_help(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_exit(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_game(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_osinfo(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_more(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_tree(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_find(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_calc(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
extern int cmd_mahjong(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_version(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);
int cmd_clear(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out);

//高斯消元法辅助函数
static int gaussian_elimination(double **matrix, int n, double *solution, FILE *out);
static int parse_linear_system(const char *system_str, double ***matrix_ptr, int *n_ptr, double **constants_ptr, FILE *out);
static int solve_linear_system(const char *system_str, FILE *out);

#endif // DECLARATIONS_H
