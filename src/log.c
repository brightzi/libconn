#include "log.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

void log_print(LOG_LEVEL level, const char *func, const char *line, const char *fmt, ...) {
    char timestamp[128];
    struct timeval nowtime;
    gettimeofday(&nowtime, NULL);
    time_t t = nowtime.tv_sec;
    struct tm currentTime;
    localtime_r(&t, &currentTime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &currentTime);

    static const char* level_strings[] = {
        "DEBUG", "INFO", "WARN", "ERROR"
    };
    
    // 打印日志头信息
    fprintf(stdout, "%s [%s] %s:%s ", timestamp, level_strings[level], func, line);
    
    // 打印用户日志内容
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}