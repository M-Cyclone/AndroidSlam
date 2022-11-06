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


    find_package(Threads REQUIRED)


    set(GLM_DIR "./external/glm/cmake/glm")
    find_package(GLM REQUIRED)


    set(OpenCV_STATIC OFF)
    set(OpenCV_DIR "./external/OpenCV-android-sdk/sdk/native/jni")
    find_package(OpenCV 4 REQUIRED)


    set(EXTERNAL_DIR ./external)


    add_subdirectory(./external)
    add_subdirectory(./app)