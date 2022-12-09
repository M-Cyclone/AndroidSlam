#pragma once
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2ext.h>  // This header should be included after GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <android_native_app_glue.h>

namespace android_slam
{

class Window
{
public:
    Window(ANativeWindow* native_window, int32_t width, int32_t height, const char* name);
    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    ~Window() noexcept;

    void resize(int32_t width, int32_t height)
    {
        m_width  = width;
        m_height = height;
        glViewport(0, 0, width, height);
    }

    void swapBuffers() const { eglSwapBuffers(m_display, m_surface); }

    int32_t            getWidth() const { return m_width; }
    int32_t            getHeight() const { return m_height; }
    const std::string& getTitle() const { return m_title; }
    float              getAspectRatio() const { return (float)m_width / (float)m_height; }

    void setWidth(int32_t value) { m_width = value; }
    void setHeight(int32_t value) { m_height = value; }
    void setTitle(std::string value) { m_title = std::move(value); }

private:
    ANativeWindow* m_native_window;  // 安卓原生窗口指针
    int32_t        m_width  = 0;     // 窗口宽度，由m_native_window获取得出
    int32_t        m_height = 0;     // 窗口高度，由m_native_window获取得出
    std::string    m_title;          // 窗口名称，随意设置

    EGLDisplay m_display;            // EGL显示设备
    EGLContext m_context;            // EGL上下文，保存了OpenGL ES的运行状态
    EGLSurface m_surface;            // EGL表面，存储了渲染结果
    EGLint     m_version_major = 0;  // EGL主版本
    EGLint     m_version_minor = 0;  // EGL副版本
};

}  // namespace android_slam