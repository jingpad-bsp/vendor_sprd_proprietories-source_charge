/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License,  Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,  software
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,  either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "common.h"
#include "cutils/properties.h"
#include "minui/minui.h"
#include "recovery_ui.h"
#include "battery.h"
#include <linux/rtc.h>
#include <sys/time.h>

#define ENG_FACOTRYMODE_FILE    "/productinfo/factorymode.file"
#define ENG_FACOTRYSYNC_FILE    "/factorysync.file"

#define RTC_TIME_FILE		"/sys/class/rtc/rtc0/time"
#define RTC_DEV_FILE		"/dev/rtc0"

extern pthread_mutex_t gBatteryMutex;
void *charge_thread(void *cookie);
void *power_thread(void *cookie);
void *input_thread(void *write_fd);
void backlight_init(void);
int is_exit = 0;
int chip_version = 0;

#define MODE_CHECK_MAX 10
#define MAX_LEN	       32

static int validate_rtc_time()
{
	int time_fd, dev_fd, ret;
	char time_buf[MAX_LEN];
	/* default rtc time 2019.1.1 00:00:00 */
	struct rtc_time tm = {
		.tm_mday = 1,
		.tm_mon = 0,
		.tm_year = 119,
	};

	time_fd = open(RTC_TIME_FILE, O_RDONLY);
	if (time_fd < 0) {
		LOGE("open rtc time file failed: %d, %s\n", time_fd, strerror(errno));
		return time_fd;
	}

	ret = read(time_fd, time_buf, sizeof(time_buf));
	if (ret > 0) {
		LOGD("read directly: time_buf = %s\n", time_buf);
		close(time_fd);
		return 0;
	}

	dev_fd = open(RTC_DEV_FILE, O_RDWR);
	if (dev_fd < 0) {
		LOGE("open /dev/rtc0 failed: %d, %s\n", dev_fd, strerror(errno));
		close(time_fd);
		return dev_fd;
	}

	ret = ioctl(dev_fd, RTC_SET_TIME, &tm);
	if (ret < 0) {
		LOGE("ioctl /dev/rtc0 failed: %d, %s\n", ret, strerror(errno));
		close(dev_fd);
		close(time_fd);
		return ret;
	}

	ret = read(time_fd, time_buf, sizeof(time_buf));
	if (ret > 0) {
		LOGD("read after set default time: time_buf = %s\n", time_buf);
		close(dev_fd);
		close(time_fd);
		return 0;
	}

	LOGD("read time_fd: ret = %d\n", ret);
	close(dev_fd);
	close(time_fd);

	return -1;
}

int
main(int argc,  char **argv) {
    time_t start = time(NULL);
    int ret,  fd = 0;
    char charge_prop[PROPERTY_VALUE_MAX] = {0};
    char ab_prop[PROPERTY_VALUE_MAX] = {0};
    int is_factory = 0;
    char factory_prop[10] = {0};
    struct stat s;
    int i = 0;

    LOGD("\n charge start\n");
    ret = battery_status_init();
    if (ret < 0)
        return -1;

    LOGD("\n charge detecting\n");
    property_get("ro.bootmode",  charge_prop,  "");
    property_get("ro.boot.auto.chipid",  ab_prop,  "");
    LOGD("charge_prop: %s\n",  charge_prop);
    LOGD("chip version: %s\n",  ab_prop);
    if(!strncmp(ab_prop, "UD710-AA", 8)){
       chip_version = 1;
    }else{
       chip_version = 0;
    }
    if (!charge_prop[0] || strncmp(charge_prop,  "charge",  6)) {
        LOGE("exit 1\n");
        return EXIT_SUCCESS;
    }

    validate_rtc_time();

    ui_init();

    LOGD("ui_init\n");

    ui_set_background(BACKGROUND_ICON_NONE);
    ui_show_indeterminate_progress();
    backlight_init();
    pthread_t t_1,  t_2,  t_3;
    ret = pthread_create(&t_1,  NULL,  charge_thread,  NULL);
    if (ret) {
        LOGE("thread:charge_thread creat failed\n");
        return -1;
    }
    ret = pthread_create(&t_2,  NULL,  input_thread,  NULL);
    if (ret) {
        LOGE("thread:input_thread creat failed\n");
        return -1;
    }

    ret = pthread_create(&t_3,  NULL,  power_thread,  NULL);
    if (ret) {
        LOGE("thread: power_thread creat failed\n");
        return -1;
    }

    LOGD("all thread start\n");

    pthread_join(t_1,  NULL);
    pthread_join(t_2,  NULL);
    pthread_join(t_3,  NULL);

    LOGD("charge app exit\n");

    return EXIT_SUCCESS;
}
