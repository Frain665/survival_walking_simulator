#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

namespace core::renderer
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() noexcept;

        Mesh(const Mesh&)            = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void upload(const std::vector<Vertex>& vertices,
                    const std::vector<GLuint>& indices);

        void draw() const noexcept;

        [[nodiscard]] bool isValid() const noexcept { return m_vao != 0; }

        static Mesh makePlane(float size, int subdivisions = 1);
        static Mesh makeCube(float halfExtent = 0.5f);

    private:
        void release() noexcept;

        GLuint m_vao        { 0 };
        GLuint m_vbo        { 0 };
        GLuint m_ebo        { 0 };
        GLsizei m_indexCount{ 0 };
    };
}
