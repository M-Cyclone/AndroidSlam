#pragma once
#include <string>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <Eigen/Eigen>

namespace android_slam
{

    class Shader
    {
    private:
        Shader(const char* vert_src, const char* frag_src);
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

    public:
        ~Shader() noexcept;

        void bind() const { glUseProgram(m_id); }
        void unbind() const { glUseProgram(0); }

        void setInt(const std::string& name, int value) const noexcept
        {
            glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
        }
        void setFloat(const std::string& name, float value) const noexcept
        {
            glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
        }
        void setVec2(const std::string& name, Eigen::Vector2f value) const noexcept
        {
            glUniform2f(glGetUniformLocation(m_id, name.c_str()), value.x(), value.y());
        }
        void setVec3(const std::string& name, Eigen::Vector3f value) const noexcept
        {
            glUniform3f(glGetUniformLocation(m_id, name.c_str()), value.x(), value.y(), value.z());
        }
        void setVec4(const std::string& name, Eigen::Vector4f value) const noexcept
        {
            glUniform4f(glGetUniformLocation(m_id, name.c_str()), value.x(), value.y(), value.z(), value.w());
        }
        void setMat2(const std::string& name, Eigen::Matrix2f matrix) const noexcept
        {
            glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, (float*)(&matrix));
        }
        void setMat3(const std::string& name, Eigen::Matrix3f matrix) const noexcept
        {
            glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, (float*)(&matrix));
        }
        void setMat4(const std::string& name, Eigen::Matrix4f matrix) const noexcept
        {
            glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, (float*)(&matrix));
        }


        static std::shared_ptr<Shader> create(const char* vert_path, const char* frag_path);

    private:
        static std::string readFile(const char* path);

    private:
        uint32_t m_id = 0;
    };

}