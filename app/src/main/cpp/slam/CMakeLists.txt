cmake_minimum_required(VERSION 3.16)

project(orbslam3)

    find_package(Threads REQUIRED)

    set(OpenCV_STATIC OFF)
    set(OpenCV_DIR "./external/OpenCV-android-sdk/sdk/native/jni")
    find_package(OpenCV 4 REQUIRED)


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
    file(GLOB slam_camera_files  CONFIGURE_DEPENDS ./include/camera_models/*.h ./src/camera_models/*.cpp)
    file(GLOB slam_core_files    CONFIGURE_DEPENDS ./include/core/*.h ./src/core/*.cpp)
    file(GLOB slam_feature_files CONFIGURE_DEPENDS ./include/feature/*.h ./src/feature/*.cpp)
    file(GLOB slam_frame_files   CONFIGURE_DEPENDS ./include/frame/*.h ./src/frame/*.cpp)
    file(GLOB slam_map_files     CONFIGURE_DEPENDS ./include/map/*.h ./src/map/*.cpp)
    file(GLOB slam_solver_files  CONFIGURE_DEPENDS ./include/solver/*.h ./src/solver/*.cpp)
    file(GLOB slam_threads_files CONFIGURE_DEPENDS ./include/threads/*.h ./src/threads/*.cpp)
    file(GLOB slam_utils_files   CONFIGURE_DEPENDS ./include/utils/*.h ./src/utils/*.cpp)

    add_library(${PROJECT_NAME} SHARED
        ${slam_camera_files}
        ${slam_core_files}
        ${slam_feature_files}
        ${slam_frame_files}
        ${slam_map_files}
        ${slam_solver_files}
        ${slam_threads_files}
        ${slam_utils_files}
    )
    target_include_directories(${PROJECT_NAME}
        PRIVATE ./include
        PRIVATE ./external/eigen3
        PRIVATE ./external/DBoW2
        PRIVATE ./external/g2o
        PRIVATE ./external/Sophus
    )
    target_link_libraries(${PROJECT_NAME}
        ${OpenCV_LIBS}
        g2o
        DBoW2
        Sophus
    )


    add_library(slam_kernel SHARED
        ./interface/SlamKernel.h
        ./interface/SlamKernel.cpp
    )
    target_include_directories(slam_kernel
        PRIVATE ./include
        PRIVATE ./external/eigen3
        PRIVATE ./external/DBoW2
        PRIVATE ./external/g2o
        PRIVATE ./external/Sophus
        PUBLIC ./interface
    )
    target_link_libraries(slam_kernel
        orbslam3
    )