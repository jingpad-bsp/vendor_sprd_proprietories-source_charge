/********************************************************************************
**  Copyright:  2016 Spreadtrum, Incorporated. All Rights Reserved.                     
*********************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "common.h"
#include <pthread.h>

static int log_fd = -1;
#define LOG_BUF_MAX 512
#define LOG_LEVEL 7
pthread_mutex_t glogMutex = PTHREAD_MUTEX_INITIALIZER;

static const char *name = "/dev/kmsg";
void log_write(int level, const char *fmt, ...) {
    char buf[LOG_BUF_MAX];
    va_list ap;

    if (level > LOG_LEVEL) return;

    pthread_mutex_lock(&glogMutex);
    log_fd = open(name, O_WRONLY);
    if (log_fd < 0) {
	pthread_mutex_unlock(&glogMutex);
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_MAX, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    va_end(ap);
    write(log_fd, buf, strlen(buf));
    close(log_fd);
    pthread_mutex_unlock(&glogMutex);
}
