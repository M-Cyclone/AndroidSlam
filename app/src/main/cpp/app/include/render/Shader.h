#pragma once
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace android_slam
{

// OpenGL ES的Shader类，具体参见LearnOpenGL网站
class Shader
{
public:
    Shader(const char* vert_path, const char* frag_path);
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;
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
    void setVec2(const std::string& name, glm::vec2 value) const noexcept
    {
        glUniform2f(glGetUniformLocation(m_id, name.c_str()), value.x, value.y);
    }
    void setVec3(const std::string& name, glm::vec3 value) const noexcept
    {
        glUniform3f(glGetUniformLocation(m_id, name.c_str()), value.x, value.y, value.z);
    }
    void setVec4(const std::string& name, glm::vec4 value) const noexcept
    {
        glUniform4f(glGetUniformLocation(m_id, name.c_str()), value.x, value.y, value.z, value.w);
    }
    void setMat2(const std::string& name, glm::mat2 matrix) const noexcept
    {
        glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
    }
    void setMat3(const std::string& name, glm::mat3 matrix) const noexcept
    {
        glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
    }
    void setMat4(const std::string& name, glm::mat4 matrix) const noexcept
    {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
    }

private:
    static std::string readFile(const char* path);

private:
    uint32_t m_id = 0;
};

}  // namespace android_slam