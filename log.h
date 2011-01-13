///////////////////////////////////////////////////////////////////////////////
//
// Name: common.h
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#ifndef __RCTLOG_H__
#define __RCTLOG_H__

#include <stdio.h>

#define LOG_D(pcString, ...) \
    do {\
        fprintf(stderr, "[DEBUG] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#define LOG_V(pcString, ...) \
    do {\
        fprintf(stderr, "[VERBOSE] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#define LOG_I(pcString, ...) \
    do {\
        fprintf(stderr, "[INFO] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#define LOG_W(pcString, ...) \
    do {\
        fprintf(stderr, "[WARNNING] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#define LOG_E(pcString, ...) \
    do {\
        fprintf(stderr, "[ERROR] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#define LOG_F(pcString, ...) \
    do {\
        fprintf(stderr, "[FATEL] %s(%d) ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

#endif


