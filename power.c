/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hardware_legacy/power.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "common.h"
#include "minui/minui.h"


#define LOG_TAG "power"

int adf_blank_done = 1;
int screen_on_flag = 1;
int flip_enter = 0;
extern int chip_version;
int autosuspend_enable();
int autosuspend_disable();
static int request_suspend(bool enable) {
    if (enable)
        return autosuspend_enable();
    else
        return autosuspend_disable();
}

int set_screen_state(int on) {
    if(chip_version){
	LOGE("chip verison is A do not sleep\n");
	return 0;
    }
    adf_blank_done = on;
    if(status_index > 0){
	return 0;
    }
    if(flip_enter){
        while(flip_enter){
		usleep(10000);
	}
    }

    LOGI("*** set_screen_state %d\n", on);
    
    if (screen_on_flag != on) {
	gr_fb_blank(!on);
	screen_on_flag = on;
    }
    
    if (!on) 
        request_suspend(true);

    return 0;
}
