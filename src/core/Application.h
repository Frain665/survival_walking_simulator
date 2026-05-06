#pragma once

#include <memory>
#include <atomic>

#include "Window.h"
#include "Input.h"
#include "audio/AudioEngine.h"
#include "renderer/Camera.h"
#include "physics/PhysicsWorld.h"
#include "entities/Player.h"

namespace core
{
	class Application final
	{
	public:
		[[nodiscard]] static auto getInstance() noexcept -> Application&
		{
			static Application instance;
			return instance;
		}

		Application(const Application&) = delete;
		Application(Application&&)      = delete;
		auto operator=(const Application&) -> Application& = delete;
		auto operator=(Application&&)      -> Application& = delete;

		auto run() -> int;

		void requestStop() noexcept { m_isRunning.store(false, std::memory_order_release); }

		[[nodiscard]] auto getWindow()       const noexcept -> const Window& { return *m_window; }
		[[nodiscard]] auto getWindow()             noexcept ->       Window& { return *m_window; }
		[[nodiscard]] auto getInput()        const noexcept -> const Input&  { return *m_input;  }
		[[nodiscard]] auto getInput()              noexcept ->       Input&  { return *m_input;  }
		[[nodiscard]] auto getCamera()       const noexcept -> const renderer::Camera& { return *m_camera; }
		[[nodiscard]] auto getCamera()             noexcept ->       renderer::Camera& { return *m_camera; }
		[[nodiscard]] auto getPhysicsWorld() const noexcept -> const physics::PhysicsWorld& { return *m_physicsWorld; }
		[[nodiscard]] auto getPhysicsWorld()       noexcept ->       physics::PhysicsWorld& { return *m_physicsWorld; }
		[[nodiscard]] auto getAudioEngine()  const noexcept -> const audio::AudioEngine& { return *m_audio; }
		[[nodiscard]] auto getAudioEngine()        noexcept ->       audio::AudioEngine& { return *m_audio; }

	private:
		Application() noexcept = default;
		~Application() noexcept { cleanup(); }

		void init();
		void cleanup() noexcept;

		void processInput(float deltaTime) noexcept;
		void update(float deltaTime) noexcept;
		void updateAudioSystem() noexcept;
		void render() const;

		// === Подсистемы ===
		std::unique_ptr<Window>                  m_window;
		std::unique_ptr<Input>                   m_input;
		std::unique_ptr<audio::AudioEngine>      m_audio;
		std::unique_ptr<renderer::Camera>        m_camera;
		std::unique_ptr<physics::PhysicsWorld>   m_physicsWorld;
		std::unique_ptr<entities::Player>        m_player;

		std::atomic<bool> m_isRunning    { false };
		bool              m_isInitialized{ false };
	};

} // namespace core
