cmake_minimum_required(VERSION 3.18.1)

project("AndroidSlam")

    add_library(NativeGlue STATIC
        ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.h
        ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    target_include_directories(NativeGlue
        PUBLIC ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue
    )
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    message("[Android Slam CMake Info] Current app is compiled with ABI Type: <${CMAKE_ANDROID_ARCH_ABI}>")

    add_subdirectory(./app)