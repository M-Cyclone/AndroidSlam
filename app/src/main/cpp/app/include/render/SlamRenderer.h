#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <SlamKernel.h>

#include "render/AABB.h"
#include "render/ImageTexture.h"
#include "render/Plane2D.h"
#include "render/Shader.h"

namespace android_slam
{

// SLAM模块的渲染器，用于渲染点云、相机轨迹、每帧图像等
class SlamRenderer
{
public:
    SlamRenderer(float fov, float aspect_ratio, float z_near, float z_far);
    SlamRenderer(const SlamRenderer&)            = delete;
    SlamRenderer& operator=(const SlamRenderer&) = delete;
    ~SlamRenderer();

    void setImage(int32_t width, int32_t height, const Image& img);
    void setData(const TrackingResult& tracking_result);

    void clearColor() const;
    void drawMapPoints(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const;
    void drawKeyFrames(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const;
    void drawImage(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const;
    void drawTotalTrajectory(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const;

    void setFov(float fov)
    {
        m_fov = fov;
        updateProj();
    }
    void setAspectRatio(float aspect_ratio)
    {
        m_aspect_ratio = aspect_ratio;
        updateProj();
    }
    void setZNearAndFar(float z_near, float z_far)
    {
        m_z_near = z_near;
        m_z_far  = z_far;
        updateProj();
    }

private:
    void updateProj() { m_proj = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_z_near, m_z_far); }

public:
    glm::vec4 m_clear_color = { 1.0f, 1.0f, 1.0f, 1.0f };  // 清屏颜色
    glm::vec3 m_mp_color    = { 0.1f, 0.1f, 0.1f };        // 地图点颜色
    glm::vec3 m_kf_color    = { 1.0f, 0.0f, 0.0f };        // 轨迹颜色

    float m_point_size = 5.0f;  // 地图点大小
    float m_line_width = 5.0f;  // 轨迹线粗细

    bool m_show_mappoints        = true;   // 是否显示地图点
    bool m_show_keyframes        = true;   // 是否显示轨迹
    bool m_show_image            = false;  // 是否显示每帧获取的图像
    bool m_show_total_trajectory = false;  // 是否显示轨迹俯视图

private:
    float m_fov          = 45.0f;
    float m_aspect_ratio = 1.0f;
    float m_z_near       = 0.1f;
    float m_z_far        = 100.0f;

    glm::mat4 m_view = glm::mat4(1.0f);
    glm::mat4 m_proj = glm::mat4(1.0f);

    Shader m_mvp_shader;  // 第一人称场景展示使用的shader

    // OpenGL ES渲染使用的对象
    uint32_t m_mp_vao{};
    uint32_t m_mp_vbo{};
    uint32_t m_mp_count{};
    uint32_t m_kf_vao{};
    uint32_t m_kf_vbo{};
    uint32_t m_kf_count{};

    AABB   m_global_aabb;        // 用于将轨迹放置于显示区域中心的AABB
    Shader m_pure_color_shader;  // 俯视图清屏用shader
    Shader m_global_shader;      // 俯视图使用的shader

    Plane2D                       m_image_painter;  // 渲染图像对象
    Shader                        m_image_shader;   // 如何对Plane2D贴图
    std::unique_ptr<ImageTexture> m_image_texture;  // 渲染哪张图像
};

}  // namespace android_slam