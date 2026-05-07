#pragma once

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <vector>

namespace core::renderer
{
    struct RenderObject
    {
        glm::mat4 transform{ 1.0f };
        glm::vec3 color    { 1.0f };
        const Mesh* mesh   { nullptr };
    };

    class Renderer
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool init();

        void beginFrame(const Camera& camera, float aspectRatio) noexcept;
        void submit(const RenderObject& obj) noexcept;
        void endFrame() noexcept;

        [[nodiscard]] bool isValid() const noexcept { return m_shader.isValid(); }

        // TODO: удалить когда будет World — тестовые меши
        [[nodiscard]] const Mesh& getPlane() const noexcept { return m_planeMesh; }
        [[nodiscard]] const Mesh& getCube()  const noexcept { return m_cubeMesh;  }

    private:
        Shader m_shader;
        Mesh   m_planeMesh;
        Mesh   m_cubeMesh;

        glm::mat4 m_view      { 1.0f };
        glm::mat4 m_projection{ 1.0f };

        std::vector<RenderObject> m_queue;

        glm::vec3 m_lightDir  { 0.4f, -1.0f, 0.6f };
        glm::vec3 m_lightColor{ 1.0f,  1.0f, 1.0f };
        glm::vec3 m_ambient   { 0.25f, 0.25f, 0.25f };
    };
}
