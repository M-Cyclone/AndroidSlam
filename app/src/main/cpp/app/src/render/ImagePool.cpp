#include "render/ImagePool.h"

namespace android_slam
{

    ImagePool::ImagePool(int32_t image_width, int32_t image_height, const char* vert_path, const char* frag_path)
        : m_sensor_camera(image_width, image_height, AIMAGE_FORMAT_YUV_420_888, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT)
        , m_width(image_width)
        , m_height(image_height)
        , m_fbo{}
        , m_color_attachment{}
        , m_depth_stencil_attachment{}
        , m_transform_painter{}
        , m_transform_shader(vert_path, frag_path)
        , m_transform_texture(image_width, image_height)
    {
        // Color attachment, where the pixels will be drawn.
        glGenTextures(1, &m_color_attachment);
        glBindTexture(GL_TEXTURE_2D, m_color_attachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Depth and stencil attachment, using render buffers because usually not need to read the buffers.
        glGenRenderbuffers(1, &m_depth_stencil_attachment);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_attachment);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, image_width, image_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Framebuffer.
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_attachment, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_stencil_attachment);

        // Check frame buffer's status and assure it's complete.
        bool check = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        assert(check && "[Android Slam Sensor Info] Framebuffer in ImagePool is not complete.");

        // Unbind framebuffer to avoid unexpected influence.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Start sensor camera capturing.
        m_sensor_camera.startCapture();
    }

    ImagePool::~ImagePool()
    {
        glDeleteRenderbuffers(1, &m_depth_stencil_attachment);
        glDeleteTextures(1, &m_color_attachment);
        glDeleteFramebuffers(1, &m_fbo);

        m_sensor_camera.stopCapture();
    }

    Image ImagePool::getImage()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Paint to framebuffer
        AImage* a_image = m_sensor_camera.getLatestImage();
        {
            m_transform_texture.setImage(a_image);

            m_transform_painter.bind();
            m_transform_shader.bind();

            glActiveTexture(GL_TEXTURE0);
            m_transform_shader.setInt("sensor_image", 0);
            m_transform_texture.bind();

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            m_transform_texture.unbind();
            m_transform_shader.unbind();
            m_transform_painter.unbind();
        }


        // Read pixels.
        std::vector<uint8_t> img(m_width * m_height * 3);
        glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, img.data());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        int64_t time_stamp;
        assert(AImage_getTimestamp(a_image, &time_stamp) == AMEDIA_OK);

        return Image{ std::move(img), time_stamp };
    }

}