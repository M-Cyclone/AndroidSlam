#pragma once
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include <SlamKernel.h>

#include "render/Shader.h"

namespace android_slam
{

    class SlamRenderer
    {
    public:
        SlamRenderer(float fov, float aspect_ratio, float z_near, float z_far);
        SlamRenderer(const SlamRenderer&) = delete;
        SlamRenderer& operator=(const SlamRenderer&) = delete;
        ~SlamRenderer();

        void setData(const TrackingResult& tracking_result);

        void draw() const;

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
            m_z_far = z_far;
            updateProj();
        }

    private:
        void updateProj()
        {
            m_proj = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_z_near, m_z_far);
        }

    private:
        float m_fov;
        float m_aspect_ratio;
        float m_z_near;
        float m_z_far;

        glm::mat4 m_view;
        glm::mat4 m_proj;

        Shader m_mvp_shader;

        uint32_t m_mp_vao;
        uint32_t m_kf_vao;

        uint32_t m_mp_vbo;
        uint32_t m_kf_vbo;

        uint32_t m_mp_count;
        uint32_t m_kf_count;
    };

}