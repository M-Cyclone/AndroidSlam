#pragma once
#include <cassert>
#include <string>

#include <media/NdkImage.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

namespace android_slam
{

// 相机传感器生成的AImage使用此类进行处理后得到RGB图像
class SensorTexture
{
public:
    SensorTexture(int32_t width, int32_t height) noexcept
        : m_width(width)
        , m_height(height)
    {
        glGenTextures(1, &m_texture);
        assert((m_texture != 0) && "[Android Slam Render Info] Failed to create GL texture.");
    }
    SensorTexture(const SensorTexture&)            = delete;
    SensorTexture& operator=(const SensorTexture&) = delete;
    ~SensorTexture() noexcept { glDeleteTextures(1, &m_texture); }

    void setImage(AImage* image);

    void bind() const { glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_texture); }
    void unbind() const { glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0); }

    int32_t getWidth() const { return m_width; }
    int32_t getHeight() const { return m_height; }

    static void registerFunctions();

private:
    // 用于使用OES扩展所引入的函数，以函数指针方式存储
    static PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID;
    static PFNEGLCREATEIMAGEKHRPROC               eglCreateImageKHR;
    static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC    glEGLImageTargetTexture2DOES;

    uint32_t m_texture = 0;
    int32_t  m_width   = 0;
    int32_t  m_height  = 0;

    AImage* m_curr_image = nullptr;  // 每帧图像的AImage指针
};

}  // namespace android_slam