# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

##
## This is the Android Makefile (?)
##


## LUA
LOCAL_PATH		:= $(call my-dir)/../..

include $(CLEAR_VARS)

LOCAL_MODULE    := lua
LOCAL_ARM_MODE	:= arm

include ../Makelist_Lua
SRCS = $(LUA_SRCS:%.c=3rdparty/Lua/lua-5.1.4/src/%.c)
LOCAL_SRC_FILES := $(SRCS)

LOCAL_CFLAGS := -g #debug
LOCAL_LDFLAGS := -Wl,-Map,xxx.map #create map file

include $(BUILD_STATIC_LIBRARY)

## Libzip
include $(CLEAR_VARS)

LOCAL_MODULE    := zip
LOCAL_ARM_MODE	:= arm

include ../Makelist_Zip
SRCS = $(ZIP_SRCS:%.c=3rdparty/libzip-0.10/lib/%.c)
LOCAL_SRC_FILES := $(SRCS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/3rdparty/libzip-0.10/lib
# We need ZLIB to compile libzip
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -lz 

LOCAL_CFLAGS := -g #debug
LOCAL_LDFLAGS := -Wl,-Map,xxx.map #create map file

include $(BUILD_STATIC_LIBRARY)

## Vitae

## Reset make state
include $(CLEAR_VARS)

LOCAL_MODULE    := vitae
include ../Makelist
LOCAL_SRC_FILES := android/jni/android.c
LOCAL_SRC_FILES	+= $(SRCS)
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -L$(SYSROOT)/usr/lib -lz 
LOCAL_STATIC_LIBRARIES := android_native_app_glue lua zip

MY_LUA_PATH := 3rdparty/Lua/lua-5.1.4/src
MY_GLFW_PATH := 3rdparty/glfw-2.7.2/include
MY_LIBZIP_PATH := 3rdparty/libzip-0.10/lib

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MY_LUA_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MY_GLFW_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MY_LIBZIP_PATH)

LOCAL_CFLAGS := -g #debug
LOCAL_LDFLAGS := -Wl,-Map,xxx.map #create map file
LOCAL_CFLAGS += -std=gnu99

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

