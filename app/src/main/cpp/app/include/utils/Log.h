#pragma once

#include <android/log.h>

#ifdef LOG
#define DEBUG_INFO(...) ::__android_log_print(ANDROID_LOG_INFO, "[Android SLAM DEBUG]", __VA_ARGS__)
#else
#define DEBUG_INFO(...)
#endif