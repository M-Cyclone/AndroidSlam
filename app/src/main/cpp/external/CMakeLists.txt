cmake_minimum_required(VERSION 3.16)
project(external_part)

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