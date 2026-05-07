#include "Application.h"
#include "Window.h"
#include "Input.h"

#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace core
{
    void Application::init()
    {
        if (m_isInitialized) [[unlikely]] return;

        try
        {
            m_window = std::make_unique<Window>("Voxel game", 1920, 1080);
            m_window->init();

            m_input = std::make_unique<Input>();
            m_input->init(*m_window);

            m_input->setCursorMode(CursorMode::Disabled);
            m_isCursorCaptured = true;

            m_audio = std::make_unique<audio::AudioEngine>();
            if (!m_audio->init())
                throw std::runtime_error("Failed to initialize audio engine");

            m_audio->setMasterVolume(0.8f);

            m_camera = std::make_unique<renderer::Camera>(
                glm::vec3{ 0.0f, 64.0f + entities::Player::EYE_HEIGHT, 0.0f }
            );

            m_renderer = std::make_unique<renderer::Renderer>();
            if (!m_renderer->init())
                throw std::runtime_error("Failed to initialize renderer");

            m_physicsWorld = std::make_unique<physics::PhysicsWorld>();

            // TODO: удалить когда будет World — временный пол 100x100
            for (int x = -50; x < 50; ++x)
            {
                for (int z = -50; z < 50; ++z)
                {
                    m_physicsWorld->addStaticCollider(
                    {
                        glm::vec3{ float(x),     63.0f, float(z)     },
                        glm::vec3{ float(x + 1), 64.0f, float(z + 1) }
                    });
                }
            }

            m_player = std::make_unique<entities::Player>(
                *m_physicsWorld,
                *m_camera,
                glm::vec3{ 0.0f, 64.0f, 0.0f }
            );

            m_isInitialized = true;
            std::cout << "[Application] Initialized successfully\n";
        }
        catch (const std::exception& e)
        {
            cleanup();
            throw std::runtime_error(std::format("Application init failed: {}", e.what()));
        }
    }

    void Application::cleanup() noexcept
    {
        const bool wasInitialized = m_isInitialized;
        m_isRunning.store(false, std::memory_order_release);

        m_player.reset();
        m_physicsWorld.reset();
        m_renderer.reset();
        m_camera.reset();
        m_audio.reset();
        m_input.reset();
        m_window.reset();

        m_isInitialized    = false;
        m_isCursorCaptured = false;

        if (wasInitialized)
            std::cout << "[Application] Cleaned up\n";
    }

    auto Application::run() -> int
    {
        try { init(); }
        catch (const std::exception& e)
        {
            std::cerr << std::format("[Application] Startup error: {}\n", e.what());
            return EXIT_FAILURE;
        }

        m_isRunning.store(true, std::memory_order_release);

        using Clock = std::chrono::high_resolution_clock;
        auto previousTime = Clock::now();
        constexpr float MAX_DELTA_TIME = 0.05f;

        std::cout << "[Application] Starting main loop\n";

        while (m_isRunning.load(std::memory_order_acquire) && !m_window->shouldClose())
        {
            const auto  currentTime   = Clock::now();
            const float frameDuration = std::chrono::duration<float>(currentTime - previousTime).count();
            previousTime = currentTime;

            const float deltaTime = std::min<float>(frameDuration, MAX_DELTA_TIME);

            processInput(deltaTime);
            update(deltaTime);
            render();

            m_window->pollEvents();
        }

        cleanup();
        return EXIT_SUCCESS;
    }

    void Application::processInput(float /*deltaTime*/) noexcept
    {
        if (!m_input) [[unlikely]] return;

        m_input->update();

        if (m_input->isKeyJustPressed(Key::Escape))
            requestStop();

        if (m_input->isKeyJustPressed(Key::F1))
        {
            m_isCursorCaptured = !m_isCursorCaptured;
            m_input->setCursorMode(m_isCursorCaptured ? CursorMode::Disabled : CursorMode::Normal);
        }
    }

    void Application::update(float deltaTime) noexcept
    {
        if (m_player && m_input)
            m_player->update(*m_input, deltaTime);

        updateAudioSystem();

        // TODO: World::update(deltaTime)
        // TODO: EntityManager::update(deltaTime)
    }

    void Application::render() const
    {
        if (!m_window || !m_window->isValid()) [[unlikely]] return;

        glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_renderer && m_camera)
        {
            const float aspect = static_cast<float>(m_window->getWidth()) /
                                 static_cast<float>(m_window->getHeight());

            m_renderer->beginFrame(*m_camera, aspect);

            // Плоскость (пол на Y=64)
            {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 64.0f, 0.0f));
                m_renderer->submit({ model, glm::vec3(0.36f, 0.65f, 0.28f), &m_renderer->getPlane() });
            }

            // TODO: удалить когда будет World — тестовые кубы
            {
                const glm::vec3 cubePositions[] = {
                    {  0.0f, 65.0f,  0.0f },
                    {  3.0f, 65.0f,  3.0f },
                    { -3.0f, 65.0f,  3.0f },
                    {  3.0f, 65.0f, -3.0f },
                    { -3.0f, 65.0f, -3.0f },
                    {  0.0f, 66.0f,  0.0f },
                };

                const glm::vec3 cubeColors[] = {
                    { 0.8f, 0.3f, 0.3f },
                    { 0.3f, 0.8f, 0.3f },
                    { 0.3f, 0.3f, 0.8f },
                    { 0.8f, 0.8f, 0.3f },
                    { 0.8f, 0.3f, 0.8f },
                    { 0.3f, 0.8f, 0.8f },
                };

                for (int i = 0; i < 6; ++i)
                {
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
                    m_renderer->submit({ model, cubeColors[i], &m_renderer->getCube() });
                }
            }

            m_renderer->endFrame();
        }

        m_window->swapBuffers();
    }

    void Application::updateAudioSystem() noexcept
    {
        if (!m_audio || !m_player || !m_camera) [[unlikely]] return;

        const glm::vec3 eyePos  = m_player->getEyePosition();
        const glm::vec3 forward = m_camera->getForward();
        const glm::vec3 up      = m_camera->getUp();

        m_audio->setListenerPosition(eyePos);
        m_audio->setListenerOrientation(forward, up);
        m_audio->update();
    }
}
