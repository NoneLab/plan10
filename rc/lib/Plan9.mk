# Data installed into the initial KFS root (not a native executable).
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := rcmain-data
LOCAL_SRC_FILES := rcmain
LOCAL_INSTALL_IMAGE := kfs
LOCAL_INSTALL_PATH := /rc/lib
include $(BUILD_INSTALL_DATA)
