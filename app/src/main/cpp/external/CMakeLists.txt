cmake_minimum_required(VERSION 3.16)
project(external_part)

    file(GLOB g2o_files CONFIGURE_DEPENDS
        ./g2o/g2o/core/*.h
        ./g2o/g2o/core/*.hpp
        ./g2o/g2o/core/*.cpp
        ./g2o/g2o/solvers/*.h
        ./g2o/g2o/stuff/*.h
        ./g2o/g2o/stuff/*.c
        ./g2o/g2o/stuff/*.cpp
        ./g2o/g2o/types/*.h
        ./g2o/g2o/types/*.cpp
    )
    add_library(g2o SHARED
        ${g2o_files}
    )
    target_include_directories(g2o
        PRIVATE ./g2o/g2o/core
        PRIVATE ./g2o/g2o/solvers
        PRIVATE ./g2o/g2o/types
        PRIVATE ./g2o/g2o/stuff
        PRIVATE ./eigen3
    )


    file(GLOB dbow2_files CONFIGURE_DEPENDS
        ./DBoW2/DBoW2/*.h
        ./DBoW2/DBoW2/*.cpp
        ./DBoW2/DUtils/*.h
        ./DBoW2/DUtils/*.cpp
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
        ./Sophus/sophus/*.hpp
        ./Sophus/sophus/*.cpp
    )
    add_library(Sophus SHARED
        ${sophus_files}
    )
    target_include_directories(Sophus
        PRIVATE ./Sophus/sophus
        PRIVATE ./eigen3
    )


    file(GLOB imgui_android_files CONFIGURE_DEPENDS
        ./imgui/*.h
        ./imgui/*.cpp
        ./imgui/backends/imgui_impl_opengl3.h
        ./imgui/backends/imgui_impl_opengl3.cpp
        ./imgui/backends/imgui_impl_android.h
        ./imgui/backends/imgui_impl_android.cpp
    )
    add_library(imgui SHARED
        ${imgui_android_files}
    )
    target_include_directories(imgui
        PUBLIC ./imgui
        PUBLIC ./imgui/backends
    )
    target_compile_definitions(imgui
        PRIVATE -DIMGUI_IMPL_OPENGL_ES3
    )
    target_link_libraries(imgui
        android
        EGL
        GLESv3
        log
    )