#pragma once
#include <type_traits>
#include <cassert>
#include <vector>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace android_slam
{

    class Model
    {
    public:
        template <typename V>
        Model(const std::vector<V>& vertices, const std::vector<uint32_t>& indices) noexcept
        {
            glGenVertexArrays(1, &m_vao);
            assert((m_vao != 0) && "[Android Slam Render Info] Failed to create VAO.");
            glBindVertexArray(m_vao);

            glGenBuffers(1, &m_vbo);
            assert((m_vbo != 0) && "[Android Slam Render Info] Failed to create VBO.");
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(V) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
            V::resolve();

            glGenBuffers(1, &m_ibo);
            assert((m_ibo != 0) && "[Android Slam Render Info] Failed to create IBO.");
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(sizeof(uint32_t) * indices.size()), indices.data(), GL_STATIC_DRAW);

            glBindVertexArray(0);
        }
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        ~Model() noexcept
        {
            glDeleteBuffers(1, &m_vbo);
            glDeleteBuffers(1, &m_ibo);
            glDeleteVertexArrays(1, &m_vao);
        }

        void bind() const { glBindVertexArray(m_vao); }
        void unbind() const { glBindVertexArray(0); }

    private:
        uint32_t m_vao = 0;
        uint32_t m_vbo = 0;
        uint32_t m_ibo = 0;
    };

}