#include "render/Shader.h"
#include <exception>

#include "utils/AssetManager.h"

namespace android_slam
{

    Shader::Shader(const char* vert_path, const char* frag_path)
    {
        std::string vert_src = readFile(vert_path);
        std::string frag_src = readFile(frag_path);
        const char* vert_c_str = vert_src.c_str();
        const char* frag_c_str = frag_src.c_str();


        GLint success = GL_FALSE;
        GLchar log_info[1024];


        uint32_t vert_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_shader, 1, &vert_c_str, nullptr);
        glCompileShader(vert_shader);

        glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            glGetShaderInfoLog(vert_shader, 1024, nullptr, log_info);
            throw std::runtime_error(log_info);
        }


        uint32_t frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_shader, 1, &frag_c_str, nullptr);
        glCompileShader(frag_shader);

        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, & success);
        if(success == GL_FALSE)
        {
            glGetShaderInfoLog(frag_shader, 1024, nullptr, log_info);
            throw std::runtime_error(log_info);
        }


        m_id = glCreateProgram();
        glAttachShader(m_id, vert_shader);
        glAttachShader(m_id, frag_shader);
        glLinkProgram(m_id);

        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if(success == GL_FALSE)
        {
            glGetProgramInfoLog(m_id, 1024, nullptr, log_info);
            throw std::runtime_error(log_info);
        }

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
    }

    Shader::~Shader() noexcept
    {
        glDeleteProgram(m_id);
    }

    std::string Shader::readFile(const char* path)
    {
        AAsset* asset = AAssetManager_open(AssetManager::get(), path, AASSET_MODE_BUFFER);
        assert(asset && "[Android Slam Shader Info] Failed to open shader file.");

        size_t size = AAsset_getLength(asset);
        auto data = (const char*) AAsset_getBuffer(asset);

        AAsset_close(asset);

        return { data, data + size };
    }

}