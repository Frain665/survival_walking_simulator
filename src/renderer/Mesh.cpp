#include "Mesh.h"

#include <utility>

namespace core::renderer
{
    Mesh::~Mesh() noexcept { release(); }

    Mesh::Mesh(Mesh&& other) noexcept
        : m_vao(std::exchange(other.m_vao, 0))
        , m_vbo(std::exchange(other.m_vbo, 0))
        , m_ebo(std::exchange(other.m_ebo, 0))
        , m_indexCount(std::exchange(other.m_indexCount, 0))
    {}

    Mesh& Mesh::operator=(Mesh&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_vao        = std::exchange(other.m_vao, 0);
            m_vbo        = std::exchange(other.m_vbo, 0);
            m_ebo        = std::exchange(other.m_ebo, 0);
            m_indexCount = std::exchange(other.m_indexCount, 0);
        }
        return *this;
    }

    void Mesh::release() noexcept
    {
        if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
        if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
        if (m_ebo) { glDeleteBuffers(1, &m_ebo);      m_ebo = 0; }
        m_indexCount = 0;
    }

    void Mesh::upload(const std::vector<Vertex>& vertices,
                      const std::vector<GLuint>& indices)
    {
        release();

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                     indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glBindVertexArray(0);

        m_indexCount = static_cast<GLsizei>(indices.size());
    }

    void Mesh::draw() const noexcept
    {
        if (!m_vao) return;
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    Mesh Mesh::makePlane(float size, int subdivisions)
    {
        std::vector<Vertex>  vertices;
        std::vector<GLuint>  indices;

        const float step = size / static_cast<float>(subdivisions);
        const float half = size * 0.5f;

        for (int z = 0; z <= subdivisions; ++z)
        {
            for (int x = 0; x <= subdivisions; ++x)
            {
                Vertex v;
                v.position = { -half + x * step, 0.0f, -half + z * step };
                v.normal   = {  0.0f, 1.0f, 0.0f };
                vertices.push_back(v);
            }
        }

        const int w = subdivisions + 1;
        for (int z = 0; z < subdivisions; ++z)
        {
            for (int x = 0; x < subdivisions; ++x)
            {
                const GLuint tl = static_cast<GLuint>(z * w + x);
                const GLuint tr = tl + 1;
                const GLuint bl = static_cast<GLuint>((z + 1) * w + x);
                const GLuint br = bl + 1;

                indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
            }
        }

        Mesh m;
        m.upload(vertices, indices);
        return m;
    }

    Mesh Mesh::makeCube(float h)
    {
        std::vector<Vertex> vertices = {
            {{ -h, -h,  h }, {  0,  0,  1 }},
            {{  h, -h,  h }, {  0,  0,  1 }},
            {{  h,  h,  h }, {  0,  0,  1 }},
            {{ -h,  h,  h }, {  0,  0,  1 }},

            {{  h, -h, -h }, {  0,  0, -1 }},
            {{ -h, -h, -h }, {  0,  0, -1 }},
            {{ -h,  h, -h }, {  0,  0, -1 }},
            {{  h,  h, -h }, {  0,  0, -1 }},

            {{ -h, -h, -h }, { -1,  0,  0 }},
            {{ -h, -h,  h }, { -1,  0,  0 }},
            {{ -h,  h,  h }, { -1,  0,  0 }},
            {{ -h,  h, -h }, { -1,  0,  0 }},

            {{  h, -h,  h }, {  1,  0,  0 }},
            {{  h, -h, -h }, {  1,  0,  0 }},
            {{  h,  h, -h }, {  1,  0,  0 }},
            {{  h,  h,  h }, {  1,  0,  0 }},

            {{ -h,  h,  h }, {  0,  1,  0 }},
            {{  h,  h,  h }, {  0,  1,  0 }},
            {{  h,  h, -h }, {  0,  1,  0 }},
            {{ -h,  h, -h }, {  0,  1,  0 }},

            {{ -h, -h, -h }, {  0, -1,  0 }},
            {{  h, -h, -h }, {  0, -1,  0 }},
            {{  h, -h,  h }, {  0, -1,  0 }},
            {{ -h, -h,  h }, {  0, -1,  0 }},
        };

        std::vector<GLuint> indices;
        for (GLuint f = 0; f < 6; ++f)
        {
            const GLuint b = f * 4;
            indices.insert(indices.end(), { b, b+1, b+2, b, b+2, b+3 });
        }

        Mesh m;
        m.upload(vertices, indices);
        return m;
    }
}
