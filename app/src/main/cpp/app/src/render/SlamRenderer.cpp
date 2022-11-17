#include "render/SlamRenderer.h"

#include "utils/Log.h"

namespace android_slam
{

    SlamRenderer::SlamRenderer(float fov, float aspect_ratio, float z_near, float z_far)
        : m_fov(fov)
        , m_aspect_ratio(aspect_ratio)
        , m_z_near(z_near)
        , m_z_far(z_far)
        , m_mvp_shader("shader/mvp.vert", "shader/mvp.frag")
        , m_pure_color_shader("shader/pure_color.vert", "shader/pure_color.frag")
        , m_global_shader("shader/global_mvp.vert", "shader/global_mvp.frag")
        , m_image_shader("shader/image2d.vert", "shader/image2d.frag")
    {
        updateProj();

        glGenVertexArrays(1, &m_mp_vao);
        glGenVertexArrays(1, &m_kf_vao);

        glGenBuffers(1, &m_mp_vbo);
        glGenBuffers(1, &m_kf_vbo);
    }

    SlamRenderer::~SlamRenderer()
    {
        glDeleteBuffers(1, &m_mp_vbo);
        glDeleteBuffers(1, &m_kf_vbo);

        glDeleteVertexArrays(1, &m_mp_vao);
        glDeleteVertexArrays(1, &m_kf_vao);
    }

    void SlamRenderer::setImage(int32_t width, int32_t height, const Image& img)
    {
        m_image_texture = std::make_unique<ImageTexture>(width, height, img.data);
    }

    void SlamRenderer::setData(const TrackingResult& tracking_result)
    {
        const auto& [last_pose, trajectory, map_points, _0, _1] = tracking_result;

        m_view = glm::mat4( last_pose[+0], last_pose[+1], -last_pose[+2], last_pose[+3]
                          , last_pose[+4], last_pose[+5], -last_pose[+6], last_pose[+7]
                          , last_pose[+8], last_pose[+9], -last_pose[10], last_pose[11]
                          , last_pose[12], last_pose[13], -last_pose[14], last_pose[15]
        );

        AABB global_aabb;

        // key frames
        {
            for(const auto [x, y, z] : trajectory)
            {
                global_aabb.addPoint({x, y, z});
            }

            m_kf_count = (uint32_t)trajectory.size();

            glBindVertexArray(m_kf_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_kf_vbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(TrackingResult::Pos) * m_kf_count), trajectory.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TrackingResult::Pos), (const void*) 0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        // map points
        {
            for(const auto [x, y, z] : map_points)
            {
                global_aabb.addPoint({x, y, z});
            }

            m_mp_count = (uint32_t)map_points.size();

            glBindVertexArray(m_mp_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_mp_vbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(TrackingResult::Pos) * m_mp_count), map_points.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TrackingResult::Pos), (const void*) 0);
            glEnableVertexAttribArray(0);
        }

        m_global_aabb = global_aabb;

        glBindVertexArray(0);
    }

    void SlamRenderer::clearColor() const
    {
        glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void SlamRenderer::drawMapPoints(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const
    {
        glViewport(x_offset, y_offset, width, height);

        if (m_show_mappoints)
        {
            m_mvp_shader.bind();
            m_mvp_shader.setMat4("u_mat_mvp", m_proj * m_view);
            m_mvp_shader.setVec3("u_color", m_mp_color);
            m_mvp_shader.setFloat("u_point_size", m_point_size);

            glBindVertexArray(m_mp_vao);
            glDrawArrays(GL_POINTS, 0, (GLsizei) m_mp_count);

            glBindVertexArray(0);
            m_mvp_shader.unbind();
        }
    }

    void SlamRenderer::drawKeyFrames(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const
    {
        glViewport(x_offset, y_offset, width, height);

        if(m_show_keyframes)
        {
            glLineWidth(m_line_width);

            m_mvp_shader.bind();
            m_mvp_shader.setMat4("u_mat_mvp", m_proj * m_view);
            m_mvp_shader.setVec3("u_color", m_kf_color);

            glBindVertexArray(m_kf_vao);
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_kf_count);

            glBindVertexArray(0);
            glLineWidth(1.0f);
            m_mvp_shader.unbind();
        }
    }

    void SlamRenderer::drawImage(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const
    {
        glViewport(x_offset, y_offset, width, height);

        if(m_show_image)
        {
            m_image_painter.bind();
            m_image_shader.bind();

            glActiveTexture(GL_TEXTURE0);
            m_image_shader.setInt("screen_shot", 0);
            m_image_texture->bind();

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            m_image_texture->unbind();
            m_image_shader.unbind();
            m_image_painter.unbind();
        }
    }

    void SlamRenderer::drawTotalTrajectory(int32_t x_offset, int32_t y_offset, int32_t width, int32_t height) const
    {
        glViewport(x_offset, y_offset, width, height);

        if(m_show_total_trajectory)
        {
            {
                m_image_painter.bind();
                m_pure_color_shader.bind();

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                m_pure_color_shader.unbind();
                m_image_painter.unbind();
            }

            {
                glLineWidth(m_line_width);


                m_global_shader.bind();
                m_global_shader.setVec3("u_center", m_global_aabb.getCenter());
                m_global_shader.setFloat("u_scale", 1.0f / m_global_aabb.getCubeMaxHalfLength());
                m_global_shader.setVec3("u_color", m_kf_color);

                glBindVertexArray(m_kf_vao);
                glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_kf_count);

                glBindVertexArray(0);
                glLineWidth(1.0f);
                m_global_shader.unbind();
            }
        }
    }

}