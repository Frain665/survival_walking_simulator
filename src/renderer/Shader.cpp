#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

namespace core::renderer
{
    Shader::~Shader() noexcept
    {
        if (m_programId)
            glDeleteProgram(m_programId);
    }

    Shader::Shader(Shader&& other) noexcept
        : m_programId(std::exchange(other.m_programId, 0))
    {}

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (this != &other)
        {
            if (m_programId) glDeleteProgram(m_programId);
            m_programId = std::exchange(other.m_programId, 0);
        }
        return *this;
    }

    bool Shader::load(std::string_view vertPath, std::string_view fragPath)
    {
        const std::string vertSrc = readFile(vertPath);
        const std::string fragSrc = readFile(fragPath);

        if (vertSrc.empty() || fragSrc.empty())
        {
            std::cerr << "[Shader] Failed to read shader files\n";
            return false;
        }

        const GLuint vert = compileStage(GL_VERTEX_SHADER,   vertSrc);
        const GLuint frag = compileStage(GL_FRAGMENT_SHADER, fragSrc);

        if (!vert || !frag)
        {
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            return false;
        }

        m_programId = glCreateProgram();
        glAttachShader(m_programId, vert);
        glAttachShader(m_programId, frag);
        glLinkProgram(m_programId);

        GLint ok = 0;
        glGetProgramiv(m_programId, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            char log[512];
            glGetProgramInfoLog(m_programId, 512, nullptr, log);
            std::cerr << "[Shader] Link error: " << log << '\n';
            glDeleteProgram(m_programId);
            m_programId = 0;
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        return m_programId != 0;
    }

    void Shader::bind()   const noexcept { glUseProgram(m_programId); }
    void Shader::unbind() const noexcept { glUseProgram(0); }

    void Shader::setInt(std::string_view name, int value) const noexcept
    {
        glUniform1i(location(name), value);
    }

    void Shader::setFloat(std::string_view name, float value) const noexcept
    {
        glUniform1f(location(name), value);
    }

    void Shader::setVec3(std::string_view name, const glm::vec3& value) const noexcept
    {
        glUniform3fv(location(name), 1, glm::value_ptr(value));
    }

    void Shader::setMat4(std::string_view name, const glm::mat4& value) const noexcept
    {
        glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    GLint Shader::location(std::string_view name) const noexcept
    {
        return glGetUniformLocation(m_programId, name.data());
    }

    GLuint Shader::compileStage(GLenum type, const std::string& src)
    {
        const GLuint id       = glCreateShader(type);
        const char*  srcPtr   = src.c_str();
        glShaderSource(id, 1, &srcPtr, nullptr);
        glCompileShader(id);

        GLint ok = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[512];
            glGetShaderInfoLog(id, 512, nullptr, log);
            std::cerr << "[Shader] Compile error: " << log << '\n';
            glDeleteShader(id);
            return 0;
        }
        return id;
    }

    std::string Shader::readFile(std::string_view path)
    {
        std::ifstream file(path.data());
        if (!file.is_open())
        {
            std::cerr << "[Shader] Cannot open: " << path << '\n';
            return {};
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
}
