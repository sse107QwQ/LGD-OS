// log.c
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "declarations.h"
// 获取当前时间字符串
static void get_time_string(char *buffer, int buffer_size) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm);
}

// 获取日志级别字符串
const char* get_log_level_string(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:   return "DEBUG";
        case LOG_LEVEL_INFO:    return "INFO";
        case LOG_LEVEL_WARNING: return "WARN";
        case LOG_LEVEL_ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

// 从字符串获取日志级别
int get_log_level_from_string(const char *level_str) {
    if (strcmp(level_str, "debug") == 0) return LOG_LEVEL_DEBUG;
    if (strcmp(level_str, "info") == 0) return LOG_LEVEL_INFO;
    if (strcmp(level_str, "warning") == 0) return LOG_LEVEL_WARNING;
    if (strcmp(level_str, "error") == 0) return LOG_LEVEL_ERROR;
    return LOG_LEVEL_INFO;  // 默认
}

// 创建日志器
Logger* create_logger(const char *filename, LogLevel level) {
    Logger *logger = (Logger*)malloc(sizeof(Logger));
    if (!logger) {
        return NULL;
    }

    // 复制文件名
    logger->filename = strdup(filename);
    if (!logger->filename) {
        free(logger);
        return NULL;
    }

    logger->file = NULL;
    logger->level = level;
    logger->max_size = 10 * 1024 * 1024;  // 10MB
    logger->rotate_count = 5;  // 保留5个备份

    // 尝试打开日志文件
    logger->file = fopen(filename, "a");
    if (!logger->file) {
        // 如果打开失败，使用标准输出
        logger->file = stdout;
    }

    return logger;
}

// 销毁日志器
void destroy_logger(Logger *logger) {
    if (logger) {
        if (logger->file && logger->file != stdout && logger->file != stderr) {
            fclose(logger->file);
        }
        if (logger->filename) {
            free(logger->filename);
        }
        free(logger);
    }
}

// 设置日志级别
void set_log_level(Logger *logger, LogLevel level) {
    if (logger) {
        logger->level = level;
    }
}

// 记录日志消息
void log_message(Logger *logger, LogLevel level, const char *format, ...) {
    if (!logger || level < logger->level) {
        return;  // 日志级别不足
    }

    char time_str[32];
    get_time_string(time_str, sizeof(time_str));

    // 格式化消息
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 写入日志
    if (logger->file) {
        fprintf(logger->file, "[%s] [%s] %s\n",
                time_str,
                get_log_level_string(level),
                buffer);
        fflush(logger->file);
    }
}
