cmake_minimum_required(VERSION 3.16)

project(android_slam)

    file(GLOB core_files   CONFIGURE_DEPENDS ./include/core/*.h ./src/core/*.cpp)
    file(GLOB render_files CONFIGURE_DEPENDS ./include/render/*.h ./src/render/*.cpp ./app/src/render/*.h)
    file(GLOB sensor_files CONFIGURE_DEPENDS ./include/sensor/*.h ./src/sensor/*.cpp)
    file(GLOB utils_files  CONFIGURE_DEPENDS ./include/utils/*.h ./src/utils/*.cpp)

    add_library(${PROJECT_NAME} SHARED
        ${core_files}
        ${render_files}
        ${sensor_files}
        ${utils_files}
    )
    target_include_directories(${PROJECT_NAME}
        PRIVATE ./include
        PRIVATE ${GLM_INCLUDE_DIRS}
        PRIVATE ${OpenCV_INCLUDE_DIRS}
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
        imgui
        ${OpenCV_LIBS}
    )
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE -DLOG
    )