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

LOCAL_PATH := $(call my-dir)/../..

include ../Makelist

include $(CLEAR_VARS)

LOCAL_MODULE    := vitae
LOCAL_SRC_FILES := android/jni/android.c
LOCAL_SRC_FILES	+= $(SRCS)
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

MY_LUA_PATH := 3rdparty/Lua/lua-5.1.4/src
MY_GLFW_PATH := 3rdparty/glfw-2.7.2/include

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MY_LUA_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(MY_GLFW_PATH)

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

