#include "render/ImagePool.h"

namespace android_slam
{

    ImagePool::ImagePool(int32_t image_width, int32_t image_height, const char* vert_path, const char* frag_path)
        : m_sensor_camera(image_width, image_height, AIMAGE_FORMAT_YUV_420_888, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT)
        , m_width(image_width)
        , m_height(image_height)
        , m_transform_painter{}
        , m_transform_shader(vert_path, frag_path)
        , m_transform_texture(image_width, image_height)
    {
        m_sensor_camera.startCapture();
    }

    ImagePool::~ImagePool()
    {
        m_sensor_camera.stopCapture();
    }

    std::vector<uint8_t> ImagePool::getImage()
    {
        // Paint to framebuffer
        {
            m_transform_texture.setImage(m_sensor_camera.getLatestImage());

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
        {

        }



        return img;
    }

}