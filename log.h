// log.h
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

// 日志级别枚举
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} LogLevel;

// Logger 结构体
typedef struct {
    char *filename;      // 日志文件名
    FILE *file;         // 日志文件指针
    LogLevel level;     // 日志级别
    int max_size;       // 最大文件大小（字节）
    int rotate_count;   // 循环日志文件数量
} Logger;

// 日志函数声明
Logger* create_logger(const char *filename, LogLevel level);
void destroy_logger(Logger *logger);
void log_message(Logger *logger, LogLevel level, const char *format, ...);
void set_log_level(Logger *logger, LogLevel level);
int get_log_level_from_string(const char *level_str);
const char* get_log_level_string(LogLevel level);

// 日志宏
#define LOG_DEBUG(logger, ...) log_message(logger, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(logger, ...)  log_message(logger, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(logger, ...)  log_message(logger, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR(logger, ...) log_message(logger, LOG_LEVEL_ERROR, __VA_ARGS__)

#endif // LOG_H
