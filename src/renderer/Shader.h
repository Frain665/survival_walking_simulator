#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <string_view>

namespace core::renderer
{
    class Shader
    {
    public:
        Shader() = default;
        ~Shader() noexcept;

        Shader(const Shader&)            = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        bool load(std::string_view vertPath, std::string_view fragPath);

        void bind()   const noexcept;
        void unbind() const noexcept;

        void setInt  (std::string_view name, int value)               const noexcept;
        void setFloat(std::string_view name, float value)             const noexcept;
        void setVec3 (std::string_view name, const glm::vec3& value)  const noexcept;
        void setMat4 (std::string_view name, const glm::mat4& value)  const noexcept;

        [[nodiscard]] bool isValid() const noexcept { return m_programId != 0; }

    private:
        static GLuint compileStage(GLenum type, const std::string& src);
        static std::string readFile(std::string_view path);

        GLint location(std::string_view name) const noexcept;

        GLuint m_programId{ 0 };
    };
}
