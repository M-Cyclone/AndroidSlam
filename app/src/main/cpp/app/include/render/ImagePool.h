#pragma once
#include <vector>

#include "sensor/SensorCamera.h"
#include "render/Plane2D.h"
#include "render/Shader.h"
#include "render/SensorTexture.h"

namespace android_slam
{

    class ImagePool
    {
    public:
        ImagePool(int32_t image_width, int32_t image_height, const char* vert_path, const char* frag_path);
        ImagePool(const ImagePool&) = delete;
        ImagePool& operator=(const ImagePool&) = delete;
        ~ImagePool();

        std::vector<uint8_t> getImage();

    private:
        SensorCamera m_sensor_camera;
        int32_t m_width;
        int32_t m_height;

        uint32_t m_fbo;
        uint32_t m_color_attachment;
        uint32_t m_depth_stencil_attachment;

        Plane2D m_transform_painter;
        Shader m_transform_shader;
        SensorTexture m_transform_texture;
    };

}