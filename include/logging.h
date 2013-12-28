#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <stdio.h>

#define LOG_LEVEL 4

#define LOG(LEVEL, LEVEL_NAME, format, ...) \
    do { \
        if(LEVEL <= LOG_LEVEL) \
        { \
            fprintf(stderr, "[%s] (%s:%d): " format "\n", LEVEL_NAME, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } \
    while(0);

#define LOG_ERROR(format, ...) LOG(2, "ERROR", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG(3, "WARN", format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG(4, "DEBUG", format, ##__VA_ARGS__)
#define LOG_DEBUG2(format, ...) LOG(5, "DEBUG", format, ##__VA_ARGS__)

#endif
