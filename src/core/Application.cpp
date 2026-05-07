#include "Application.h"
#include "Window.h"
#include "Input.h"

#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <format>
#include <iostream>
#include <stdexcept>



namespace core
{
	void Application::init()
	{
		if (m_isInitialized) [[unlikely]] return;

		try
		{
			// --- Окно и ввод ---
			m_window = std::make_unique<Window>("Voxel game", 1920, 1080);
			m_window->init();

			m_input = std::make_unique<Input>();
			m_input->init(*m_window);

			// Захватываем курсор сразу (FPS-режим)
			m_input->setCursorMode(CursorMode::Disabled);
			m_isCursorCaptured = true;

			// --- Аудио ---
			m_audio = std::make_unique<audio::AudioEngine>();
			if (!m_audio->init())
				throw std::runtime_error("Failed to initialize audio engine");

			m_audio->setMasterVolume(0.8f);

			// --- Камера ---
			m_camera = std::make_unique<renderer::Camera>(
				glm::vec3{ 0.0f, 64.0f + entities::Player::EYE_HEIGHT, 0.0f }
			);

			// --- Физика ---
			m_physicsWorld = std::make_unique<physics::PhysicsWorld>();

			// TODO: когда будет готов World/Chunk — регистрировать блоки через:
			//   m_physicsWorld->addStaticCollider(blockAABB);

			// Временно: пол на Y=64 (плоскость 100×100 блоков)
			// Удалить когда появится генератор мира
			for (int x = -50; x < 50; ++x)
			{
				for (int z = -50; z < 50; ++z)
				{
					m_physicsWorld->addStaticCollider(
					{
						glm::vec3{ float(x),      63.0f, float(z)     },
						glm::vec3{ float(x + 1),  64.0f, float(z + 1) }
					});
				}
			}

			// --- Игрок ---
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
		m_camera.reset();
		m_audio.reset();
		m_input.reset();
		m_window.reset();

		m_isInitialized = false;
		m_isCursorCaptured = true;

		if (wasInitialized)
			std::cout << "[Application] Cleaned up\n";
	}

	auto Application::run() -> int
	{
		try
		{
			init();
		}
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

			const float deltaTime = std::min(frameDuration, MAX_DELTA_TIME);

			processInput(deltaTime);
			update(deltaTime);
			render();

			m_window->pollEvents();
		}

		cleanup();
		return EXIT_SUCCESS;
	}

	// -----------------------------------------------------------------------
	// processInput — только системные нажатия; движение передаётся в Player
	// -----------------------------------------------------------------------
	void Application::processInput(float /*deltaTime*/) noexcept
	{
		if (!m_input) [[unlikely]] return;

		m_input->update();

		if (m_input->isKeyJustPressed(Key::Escape))
			requestStop();

		// F1 — переключение режима курсора (меню / игра)
		if (m_input->isKeyJustPressed(Key::F1))
		{
			m_isCursorCaptured = !m_isCursorCaptured;
			m_input->setCursorMode(m_isCursorCaptured ? CursorMode::Disabled : CursorMode::Normal);
		}
	}

	// -----------------------------------------------------------------------
	// update
	// -----------------------------------------------------------------------
	void Application::update(float deltaTime) noexcept
	{
		// Обновляем игрока (движение + физика + камера)
		if (m_player && m_input)
			m_player->update(*m_input, deltaTime);

		updateAudioSystem();

		// TODO: World::update(deltaTime)
		// TODO: EntityManager::update(deltaTime)
	}

	// -----------------------------------------------------------------------
	// render
	// -----------------------------------------------------------------------
	void Application::render() const
	{
		if (!m_window || !m_window->isValid()) [[unlikely]] return;

		// Здесь будет:
		//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//   m_renderer->beginFrame(*m_camera);
		//   m_world->render(*m_renderer);
		m_window->swapBuffers();
	}

	// -----------------------------------------------------------------------
	// updateAudioSystem — синхронизация слушателя с позицией игрока
	// -----------------------------------------------------------------------
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

} // namespace core
