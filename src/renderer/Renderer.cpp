#include "Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace core::renderer
{
    bool Renderer::init()
    {
        if (!m_shader.load("assets/shaders/basic.vert", "assets/shaders/basic.frag"))
        {
            std::cerr << "[Renderer] Failed to load shaders\n";
            return false;
        }

        m_planeMesh = Mesh::makePlane(100.0f, 10);
        m_cubeMesh  = Mesh::makeCube(0.5f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        std::cout << "[Renderer] Initialized\n";
        return true;
    }

    void Renderer::beginFrame(const Camera& camera, float aspectRatio) noexcept
    {
        m_view       = camera.getViewMatrix();
        m_projection = camera.getProjectionMatrix(aspectRatio);
        m_queue.clear();
    }

    void Renderer::submit(const RenderObject& obj) noexcept
    {
        if (obj.mesh) m_queue.push_back(obj);
    }

    void Renderer::endFrame() noexcept
    {
        m_shader.bind();
        m_shader.setMat4("uView",       m_view);
        m_shader.setMat4("uProjection", m_projection);
        m_shader.setVec3("uLightDir",   m_lightDir);
        m_shader.setVec3("uLightColor", m_lightColor);
        m_shader.setVec3("uAmbient",    m_ambient);

        for (const auto& obj : m_queue)
        {
            m_shader.setMat4("uModel", obj.transform);
            m_shader.setVec3("uColor", obj.color);
            obj.mesh->draw();
        }

        m_shader.unbind();
    }
}
