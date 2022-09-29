#pragma once
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2ext.h> // This header should be included after GLES3

#include <android_native_app_glue.h>

namespace android_slam
{

    class Window
    {
    public:
        Window(ANativeWindow* native_window, int32_t width, int32_t height, const char* name);
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        ~Window() noexcept;

        void resize(int32_t width, int32_t height)
        {
            m_width = width;
            m_height = height;
            glViewport(0, 0, width, height);
        }

        void swapBuffers() const { eglSwapBuffers(m_display, m_surface); }

        int32_t getWidth() const { return m_width; }
        int32_t getHeight() const { return m_height; }
        const std::string& getTitle() const { return m_title; }
        float getAspectRatio() const { return (float)m_width / (float)m_height; }

        void setWidth(int32_t value) { m_width = value; }
        void setHeight(int32_t value) { m_height = value; }
        void setTitle(std::string value) { m_title = std::move(value); }

    private:
        ANativeWindow* m_native_window;
        int32_t m_width = 0;
        int32_t m_height = 0;
        std::string m_title;

        EGLDisplay m_display;
        EGLContext m_context;
        EGLSurface m_surface;
        EGLint m_version_major = 0;
        EGLint m_version_minor = 0;
    };

}