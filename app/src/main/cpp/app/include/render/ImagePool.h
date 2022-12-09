#pragma once
#include <vector>

#include <SlamKernel.h>

#include "render/Plane2D.h"
#include "render/SensorTexture.h"
#include "render/Shader.h"
#include "sensor/SensorCamera.h"

namespace android_slam
{

class ImagePool
{
public:
    ImagePool(int32_t image_width, int32_t image_height, const char* vert_path, const char* frag_path);
    ImagePool(const ImagePool&)            = delete;
    ImagePool& operator=(const ImagePool&) = delete;
    ~ImagePool();

    Image getImage();

private:
    SensorCamera m_sensor_camera;  // 安卓相机对象
    int32_t      m_width;
    int32_t      m_height;

    uint32_t m_fbo;                       // 使用OpenGL ES转换所需的Frame buffer对象
    uint32_t m_color_attachment;          // FBO的颜色附件
    uint32_t m_depth_stencil_attachment;  // FBO的深度和模板附件

    Plane2D       m_transform_painter;  // 用于渲染的对象
    Shader        m_transform_shader;   // 解释如何渲染的shader
    SensorTexture m_transform_texture;  // 需要渲染的图像，这里为YUV格式
};

}  // namespace android_slam