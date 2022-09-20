#include "core/Window.h"
#include <cassert>

namespace android_slam
{

    Window::Window(ANativeWindow* native_window, int32_t width, int32_t height, const char* name)
        : m_native_window(native_window)
        , m_width(width)
        , m_height(height)
        , m_title(name)
    {
        EGLBoolean check = EGL_FALSE;

        ANativeWindow_acquire(m_native_window);


        m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        assert((m_display != EGL_NO_DISPLAY) && "[Android Slam Window Info] Failed to create Display.");
        check = eglInitialize(m_display, &m_version_major, &m_version_minor);
        assert((check == EGL_TRUE) && "[Android Slam Window Info] Failed to init EGL.");


        const EGLint config_attributes[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
        };
        EGLint config_count = 0;
        check = eglChooseConfig(m_display, config_attributes, nullptr, 0, &config_count);
        assert((check == EGL_TRUE) && "[Android Slam Window Info] Failed to choose an egl config.");
        assert((config_count > 0) && "[Android Slam Window Info] No matching config.");
        EGLConfig config;
        check = eglChooseConfig(m_display, config_attributes, &config, 1, &config_count);
        assert((check == EGL_TRUE) && "[Android Slam Window Info] Failed to choose an egl config.");


        EGLint format;
        check = eglGetConfigAttrib(m_display, config, EGL_NATIVE_VISUAL_ID, &format);
        assert((check == EGL_TRUE) && "[Android Slam Window Info] Failed to get format.");
        ANativeWindow_setBuffersGeometry(m_native_window, 0, 0, format);


        const EGLint context_attributes[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
        };
        m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, context_attributes);
        assert((m_context != EGL_NO_CONTEXT) && "[Android Slam Window Info] Failed to create Context.");


        m_surface = eglCreateWindowSurface(m_display, config, m_native_window, nullptr);
        assert((m_surface != EGL_NO_SURFACE) && "[Android Slam Window Info] Failed to create Surface.");
        eglMakeCurrent(m_display, m_surface, m_surface, m_context);


        resize(m_width, m_height);
    }

    Window::~Window() noexcept
    {
        if(m_display != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if(m_context != EGL_NO_CONTEXT) eglDestroyContext(m_display, m_context);
            if(m_surface != EGL_NO_SURFACE) eglDestroySurface(m_display, m_surface);

            eglTerminate(m_display);
        }

        m_display = EGL_NO_DISPLAY;
        m_context = EGL_NO_CONTEXT;
        m_surface = EGL_NO_SURFACE;

        ANativeWindow_release(m_native_window);
    }

}