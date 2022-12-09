#pragma once
#include <cassert>
#include <vector>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

namespace android_slam
{

// 读取一个未压缩的RGB图像作为OpenGL ES纹理
class ImageTexture
{
public:
    ImageTexture(int32_t width, int32_t height, const std::vector<uint8_t>& img) noexcept
        : m_width(width)
        , m_height(height)
    {
        glGenTextures(1, &m_texture);
        assert((m_texture != 0) && "[Android Slam Render Info] Failed to create GL texture.");

        glBindTexture(GL_TEXTURE_2D, m_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    ImageTexture(const ImageTexture&)            = delete;
    ImageTexture& operator=(const ImageTexture&) = delete;
    ~ImageTexture() noexcept { glDeleteTextures(1, &m_texture); }

    void bind() const { glBindTexture(GL_TEXTURE_2D, m_texture); }
    void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

    int32_t getWidth() const { return m_width; }
    int32_t getHeight() const { return m_height; }

private:
    uint32_t m_texture = 0;
    int32_t  m_width   = 0;
    int32_t  m_height  = 0;
};

}  // namespace android_slam