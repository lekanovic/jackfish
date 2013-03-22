LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := \
	src/main.c \
	src/utils_tree.c \
	src/parser.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/inc

LOCAL_MODULE := jackfish
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


