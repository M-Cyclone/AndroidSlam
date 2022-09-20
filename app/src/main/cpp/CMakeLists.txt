cmake_minimum_required(VERSION 3.18.1)

project("AndroidSlam")


message("[Android Slam CMake Info] Current app is compiled with ABI Type: <${CMAKE_ANDROID_ARCH_ABI}>")


add_library(NativeGlue STATIC
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.h
    ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(NativeGlue
    PUBLIC ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue
)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")


find_package(Threads REQUIRED)

set(OpenCV_STATIC OFF)
set(OpenCV_DIR "./external/OpenCV-android-sdk/sdk/native/jni")
find_package(OpenCV 4 REQUIRED)


# slam's external libs
file(GLOB g2o_files CONFIGURE_DEPENDS
    ./external/g2o/g2o/core/*.h
    ./external/g2o/g2o/core/*.hpp
    ./external/g2o/g2o/core/*.cpp
    ./external/g2o/g2o/solvers/*.h
    ./external/g2o/g2o/stuff/*.h
    ./external/g2o/g2o/stuff/*.c
    ./external/g2o/g2o/stuff/*.cpp
    ./external/g2o/g2o/types/*.h
    ./external/g2o/g2o/types/*.cpp
)
add_library(g2o SHARED
    ${g2o_files}
)
target_include_directories(g2o
    PRIVATE ./external/g2o/g2o/core
    PRIVATE ./external/g2o/g2o/solvers
    PRIVATE ./external/g2o/g2o/types
    PRIVATE ./external/g2o/g2o/stuff
    PRIVATE ./external/eigen3
)


file(GLOB dbow2_files CONFIGURE_DEPENDS
    ./external/DBoW2/DBoW2/*.h
    ./external/DBoW2/DBoW2/*.cpp
    ./external/DBoW2/DUtils/*.h
    ./external/DBoW2/DUtils/*.cpp
)
add_library(DBoW2 SHARED
    ${dbow2_files}
)
target_include_directories(DBoW2
    PUBLIC ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(DBoW2
    ${OpenCV_LIBS}
)


file(GLOB sophus_files CONFIGURE_DEPENDS
    ./external/Sophus/sophus/*.hpp
    ./external/Sophus/sophus/*.cpp
)
add_library(Sophus SHARED
    ${sophus_files}
)
target_include_directories(Sophus
    PRIVATE ./external/Sophus/sophus
    PRIVATE ./external/eigen3
)


# orbslam3
file(GLOB slam_camera_files  CONFIGURE_DEPENDS ./slam/include/camera_models/*.h ./slam/src/camera_models/*.cpp)
file(GLOB slam_core_files    CONFIGURE_DEPENDS ./slam/include/core/*.h ./slam/src/core/*.cpp)
file(GLOB slam_feature_files CONFIGURE_DEPENDS ./slam/include/feature/*.h ./slam/src/feature/*.cpp)
file(GLOB slam_frame_files   CONFIGURE_DEPENDS ./slam/include/frame/*.h ./slam/src/frame/*.cpp)
file(GLOB slam_map_files     CONFIGURE_DEPENDS ./slam/include/map/*.h ./slam/src/map/*.cpp)
file(GLOB slam_solver_files  CONFIGURE_DEPENDS ./slam/include/solver/*.h ./slam/src/solver/*.cpp)
file(GLOB slam_threads_files CONFIGURE_DEPENDS ./slam/include/threads/*.h ./slam/src/threads/*.cpp)
file(GLOB slam_utils_files   CONFIGURE_DEPENDS ./slam/include/utils/*.h ./slam/src/utils/*.cpp)

add_library(slam SHARED
    ${slam_camera_files}
    ${slam_core_files}
    ${slam_feature_files}
    ${slam_frame_files}
    ${slam_map_files}
    ${slam_solver_files}
    ${slam_threads_files}
    ${slam_utils_files}
)
target_include_directories(slam
    PUBLIC ./slam/include
    PUBLIC ./external/eigen3
    PUBLIC ./external/DBoW2
    PUBLIC ./external/g2o
    PUBLIC ./external/Sophus
)
target_link_libraries(slam
    ${OpenCV_LIBS}
    g2o
    DBoW2
    Sophus
)


# main lib
file(GLOB core_files   CONFIGURE_DEPENDS ./app/include/core/*.h ./app/src/core/*.cpp)
file(GLOB render_files CONFIGURE_DEPENDS ./app/include/render/*.h ./app/src/render/*.cpp ./app/src/render/*.h)
file(GLOB sensor_files CONFIGURE_DEPENDS ./app/include/sensor/*.h ./app/src/sensor/*.cpp)
file(GLOB utils_files  CONFIGURE_DEPENDS ./app/include/utils/*.h ./app/src/utils/*.cpp)

add_library(${PROJECT_NAME} SHARED
    ${core_files}
    ${render_files}
    ${sensor_files}
    ${utils_files}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ./external/eigen3
    PUBLIC ./app/include
)
target_link_libraries(${PROJECT_NAME}
    -landroid
    log
    NativeGlue
    Threads::Threads
    camera2ndk
    mediandk
    EGL
    GLESv3

    slam
)