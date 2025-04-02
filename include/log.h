#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif 
    
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

typedef enum LOG_LEVEL {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
}LOG_LEVEL;

void log_print(LOG_LEVEL level, const char *func, const char *line, const char *fmt, ...);

#define LOG_D(fmt, ...) log_print(LOG_LEVEL_DEBUG, __FUNCTION__, TOSTRING(__LINE__), fmt, ##__VA_ARGS__);
#define LOG_I(fmt, ...) log_print(LOG_LEVEL_INFO, __FUNCTION__, TOSTRING(__LINE__), fmt, ##__VA_ARGS__);
#define LOG_W(fmt, ...) log_print(LOG_LEVEL_WARN, __FUNCTION__, TOSTRING(__LINE__), fmt, ##__VA_ARGS__);
#define LOG_E(fmt, ...) log_print(LOG_LEVEL_ERROR, __FUNCTION__, TOSTRING(__LINE__),  fmt, ##__VA_ARGS__);

#ifdef __cplusplus
}
#endif

#endif