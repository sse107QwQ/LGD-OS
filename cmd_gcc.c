// cmd_gcc.c - GCC编译器命令实现
#include "declarations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEPARATOR ';'
#else
#include <unistd.h>
#include <sys/stat.h>
#define PATH_SEPARATOR ':'
#endif

// 检查gcc是否安装
int check_gcc_installed(FILE *out) {
    int installed = 0;

    #ifdef _WIN32
        // Windows: 检查gcc.exe是否存在
        int result = system("where gcc >nul 2>nul");
        if (result == 0) {
            installed = 1;
        } else {
            // 检查MinGW
            result = system("where mingw32-make >nul 2>nul");
            if (result == 0) {
                installed = 2;  // MinGW
            }
        }
    #else
        // Linux/macOS
        int result = system("which gcc >/dev/null 2>&1");
        if (result == 0) {
            installed = 1;
        }
    #endif

    if (!installed) {
        fprintf(out, "\n错误: 未找到 GCC 编译器！\n");
        fprintf(out, "========================================\n");
        fprintf(out, "GCC 编译器未安装或不在系统 PATH 中。\n\n");
        fprintf(out, "安装方法:\n");
        fprintf(out, "1. Windows:\n");
        fprintf(out, "   - 下载 MinGW: https://sourceforge.net/projects/mingw/\n");
        fprintf(out, "   - 或下载 MSYS2: https://www.msys2.org/\n");
        fprintf(out, "   - 安装后，将 gcc.exe 所在目录添加到 PATH 环境变量\n\n");
        fprintf(out, "2. Linux:\n");
        fprintf(out, "   - Debian/Ubuntu: sudo apt-get install gcc\n");
        fprintf(out, "   - Fedora: sudo dnf install gcc\n");
        fprintf(out, "   - Arch: sudo pacman -S gcc\n\n");
        fprintf(out, "3. macOS:\n");
        fprintf(out, "   - 安装 Xcode Command Line Tools:\n");
        fprintf(out, "     xcode-select --install\n");
        fprintf(out, "========================================\n");
    }

    return installed;
}

// 显示帮助信息
void show_gcc_help(FILE *out) {
    fprintf(out, "\nGCC编译器命令帮助:\n");
    fprintf(out, "========================================\n");
    fprintf(out, "用法: gcc [选项] <源文件> [参数]\n\n");

    fprintf(out, "主要选项:\n");
    fprintf(out, "  -h, --help       显示此帮助信息\n");
    fprintf(out, "  -v, --version    显示GCC版本\n");
    fprintf(out, "  -c               只编译不链接，生成目标文件(.o)\n");
    fprintf(out, "  -o <文件>        指定输出文件名\n");
    fprintf(out, "  -g               生成调试信息\n");
    fprintf(out, "  -O0              不优化（默认）\n");
    fprintf(out, "  -O1              基本优化\n");
    fprintf(out, "  -O2              高级优化\n");
    fprintf(out, "  -O3              最高级优化\n");
    fprintf(out, "  -Wall            启用所有警告\n");
    fprintf(out, "  -Werror          将警告视为错误\n");
    fprintf(out, "  -std=c99         使用C99标准\n");
    fprintf(out, "  -std=c11         使用C11标准\n");
    fprintf(out, "  -std=c17         使用C17标准\n");
    fprintf(out, "  -I<目录>         添加头文件搜索路径\n");
    fprintf(out, "  -L<目录>         添加库文件搜索路径\n");
    fprintf(out, "  -l<库名>         链接指定库\n");
    fprintf(out, "  -D<宏>          定义预处理器宏\n");

    fprintf(out, "\n常用示例:\n");
    fprintf(out, "  gcc hello.c                   编译hello.c\n");
    fprintf(out, "  gcc -o hello hello.c          编译hello.c，输出hello.exe\n");
    fprintf(out, "  gcc -c hello.c                只编译，生成hello.o\n");
    fprintf(out, "  gcc hello.o -o hello          链接目标文件\n");
    fprintf(out, "  gcc -Wall -g hello.c          启用所有警告和调试信息\n");
    fprintf(out, "  gcc -std=c11 hello.c          使用C11标准\n");

    fprintf(out, "\n虚拟文件系统支持:\n");
    fprintf(out, "  gcc /home/user/hello.c        编译虚拟文件系统中的文件\n");
    fprintf(out, "========================================\n");
}

// 显示GCC版本
void show_gcc_version(FILE *out) {
    fprintf(out, "\n");
    fprintf(out, "正在检测GCC版本...\n");
    fprintf(out, "--------------------------------\n");
    system("gcc --version");
    fprintf(out, "--------------------------------\n");
}

// 构建编译命令
int build_gcc_command(char *command, int max_len, int argc, char *argv[], ShellContext *ctx, FILE *out) {
    char *cmd_ptr = command;
    int remaining = max_len;

    // 声明变量
    char real_path[MAX_PATH_LEN];
    char test_path[MAX_PATH_LEN];
    char virtual_path[MAX_PATH_LEN];
    char current_dir[MAX_PATH_LEN];
    char real_output_path[MAX_PATH_LEN];
    char virtual_output_path[MAX_PATH_LEN];

    // 添加gcc命令
    int written = snprintf(cmd_ptr, remaining, "gcc");
    if (written < 0 || written >= remaining) {
        return 0;
    }
    cmd_ptr += written;
    remaining -= written;

    int has_source_file = 0;
    int output_specified = 0;
    int check_only = 0;

    // 解析参数
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // 处理选项
            if (strcmp(argv[i], "-o") == 0) {
                output_specified = 1;
                if (i + 1 < argc) {
                    char *output_file = argv[i + 1];

                    // 处理输出文件路径
                    if (output_file[0] == '/') {
                        // 虚拟路径，转换为真实路径
                        virtual_to_real_path(output_file, real_output_path);
                    } else if (strchr(output_file, '\\') != NULL ||
                               (strlen(output_file) > 1 && output_file[1] == ':')) {
                        // Windows 绝对路径，直接使用
                        strcpy(real_output_path, output_file);
                    } else {
                        // 相对路径，转换为虚拟文件系统的路径
                        if (ctx->current_dir[0] == '/') {
                            snprintf(virtual_output_path, sizeof(virtual_output_path),
                                    "%s/%s", ctx->current_dir, output_file);
                        } else {
                            snprintf(virtual_output_path, sizeof(virtual_output_path),
                                    "/%s/%s", ctx->current_dir, output_file);
                        }

                        virtual_to_real_path(virtual_output_path, real_output_path);
                    }

                    // 在 Windows 下，如果输出文件没有扩展名，添加 .exe
                    #ifdef _WIN32
                    char *ext = strrchr(real_output_path, '.');
                    if (ext == NULL || (strcmp(ext, ".exe") != 0 &&
                        strcmp(ext, ".o") != 0 && strcmp(ext, ".obj") != 0 &&
                        strcmp(ext, ".a") != 0 && strcmp(ext, ".so") != 0 &&
                        strcmp(ext, ".dll") != 0)) {
                        strcat(real_output_path, ".exe");
                    }
                    #endif

                    written = snprintf(cmd_ptr, remaining, " -o \"%s\"", real_output_path);
                    if (written < 0 || written >= remaining) {
                        return 0;
                    }
                    cmd_ptr += written;
                    remaining -= written;
                    i++;  // 跳过输出文件名
                }
            } else if (strcmp(argv[i], "-c") == 0) {
                check_only = 1;
                written = snprintf(cmd_ptr, remaining, " -c");
                if (written < 0 || written >= remaining) {
                    return 0;
                }
                cmd_ptr += written;
                remaining -= written;
            } else {
                // 其他选项
                written = snprintf(cmd_ptr, remaining, " %s", argv[i]);
                if (written < 0 || written >= remaining) {
                    return 0;
                }
                cmd_ptr += written;
                remaining -= written;
            }
        } else {
            // 处理文件参数
            char *filename = argv[i];

            // 检查文件扩展名
            char *ext = strrchr(filename, '.');
            int is_source_file = 0;
            if (ext) {
                if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
                    strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0 ||
                    strcmp(ext, ".C") == 0) {
                    is_source_file = 1;
                }
            }

            if (is_source_file) {
                has_source_file = 1;

                // 检查是否是虚拟路径
                if (filename[0] == '/') {
                    virtual_to_real_path(filename, real_path);

                    // 检查文件是否存在
                    FILE *f = fopen(real_path, "r");
                    if (f) {
                        fclose(f);
                        filename = real_path;
                    } else {
                        fprintf(out, "错误: 源文件不存在: %s\n", argv[i]);
                        fprintf(out, "真实路径: %s\n", real_path);
                        return 0;
                    }
                } else if (strstr(filename, "\\") == NULL && strstr(filename, "/") == NULL) {
                    // 可能是当前目录的相对路径
                    getcwd(current_dir, sizeof(current_dir));

                    snprintf(test_path, sizeof(test_path), "%s\\%s", current_dir, filename);

                    FILE *f = fopen(test_path, "r");
                    if (f) {
                        fclose(f);
                        filename = test_path;
                    } else {
                        // 尝试虚拟路径
                        if (ctx->current_dir[0] == '/') {
                            snprintf(virtual_path, sizeof(virtual_path), "%s/%s",
                                    ctx->current_dir, filename);
                        } else {
                            snprintf(virtual_path, sizeof(virtual_path), "/%s/%s",
                                    ctx->current_dir, filename);
                        }

                        virtual_to_real_path(virtual_path, real_path);

                        f = fopen(real_path, "r");
                        if (f) {
                            fclose(f);
                            filename = real_path;
                        }
                    }
                }

                // 添加文件名到命令
                written = snprintf(cmd_ptr, remaining, " \"%s\"", filename);
                if (written < 0 || written >= remaining) {
                    return 0;
                }
                cmd_ptr += written;
                remaining -= written;
            } else {
                // 非源文件参数（如库文件等）
                written = snprintf(cmd_ptr, remaining, " %s", filename);
                if (written < 0 || written >= remaining) {
                    return 0;
                }
                cmd_ptr += written;
                remaining -= written;
            }
        }
    }

    if (!has_source_file) {
        fprintf(out, "错误: 没有指定源文件！\n");
        return 0;
    }

    return 1;
}

// 主命令函数
int cmd_gcc(ShellContext *ctx, int argc, char *argv[], FILE *in, FILE *out) {
    (void)in;

    // 检查是否请求帮助
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            show_gcc_help(out);
            return 0;
        } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            int installed = check_gcc_installed(out);
            if (installed) {
                show_gcc_version(out);
            }
            return installed ? 0 : 1;
        }
    }

    // 检查gcc是否安装
    int installed = check_gcc_installed(out);
    if (!installed) {
        return 1;
    }

    // 构建编译命令
    char command[4096];
    if (!build_gcc_command(command, sizeof(command), argc, argv, ctx, out)) {
        return 1;
    }

    // 显示要执行的命令
    fprintf(out, "\n执行命令: %s\n", command);
    fprintf(out, "--------------------------------\n");

    // 记录开始时间
    clock_t start_time = clock();

    // 执行编译命令
    int result = system(command);

    // 记录结束时间
    clock_t end_time = clock();
    double elapsed_seconds = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(out, "--------------------------------\n");
    fprintf(out, "编译完成，用时: %.3f秒\n", elapsed_seconds);

    if (result == 0) {
        fprintf(out, "编译成功！\n");

        // 在编译成功后，尝试查找生成的文件
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
                char *output_name = argv[i + 1];
                fprintf(out, "输出文件: %s\n", output_name);
                break;
            }
        }
    } else {
        fprintf(out, "编译失败，返回码: %d\n", result);
    }

    return result;
}
