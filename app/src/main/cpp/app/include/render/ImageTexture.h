#pragma once
#include <cstdint>
#include <cassert>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace android_slam
{

    class ImageTexture
    {
    public:
        ImageTexture(int32_t width, int32_t height) noexcept
            : m_width(width)
            , m_height(height)
        {
            glGenTextures(1, &m_texture);
            assert((m_texture != 0) && "[Android Slam Render Info] Failed to create GL texture.");
            glBindTexture(GL_TEXTURE_2D, m_texture);
        }
        ImageTexture(const ImageTexture&) = delete;
        ImageTexture& operator=(const ImageTexture&) = delete;
        ~ImageTexture() noexcept
        {
            glDeleteTextures(1, &m_texture);
        }

        void setImage(const float* data)
        {
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_FLOAT, data);
        }

        void bind() const { glBindTexture(GL_TEXTURE_2D, m_texture); }
        void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

        int32_t getWidth() const { return m_width; }
        int32_t getHeight() const { return m_height; }

    private:
        uint32_t m_texture = 0;
        int32_t m_width = 0;
        int32_t m_height = 0;
    };

}