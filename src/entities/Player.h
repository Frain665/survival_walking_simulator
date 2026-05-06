#pragma once

#include <glm/glm.hpp>

#include "physics/PhysicsWorld.h"
#include "renderer/Camera.h"
#include "core/Input.h"

namespace core::entities
{
	/**
	 * @brief Игрок — связующее звено между Input, Camera и PhysicsWorld.
	 *
	 * Логика update():
	 *   1. Читаем ввод → желаемая горизонтальная скорость
	 *   2. Применяем гравитацию к вертикальной скорости
	 *   3. sweepMove через PhysicsWorld → фактическое смещение
	 *   4. Обновляем позицию и передаём её в Camera
	 *
	 * AABB игрока: ширина 0.6, высота 1.8, центрирован по X/Z.
	 * Позиция хранится как нижняя точка (ноги) — удобно для onGround.
	 */
	class Player
	{
	public:
		// Физические константы
		static constexpr float PLAYER_WIDTH  = 0.6f;
		static constexpr float PLAYER_HEIGHT = 1.8f;
		static constexpr float EYE_HEIGHT    = 1.62f; // от ног до глаз

		static constexpr float WALK_SPEED    = 4.3f;  // м/с (как в Minecraft)
		static constexpr float SPRINT_SPEED  = 5.6f;
		static constexpr float JUMP_VELOCITY = 8.4f;
		static constexpr float AIR_CONTROL   = 0.2f;  // коэфф. управления в воздухе

		// -------------------------------------------------------

		explicit Player(physics::PhysicsWorld& physicsWorld,
		                renderer::Camera&      camera,
		                glm::vec3              spawnPos = { 0.0f, 64.0f, 0.0f }) noexcept;

		/**
		 * @brief Основное обновление. Вызывать из Application::update каждый кадр.
		 * @param input      система ввода (const — Player только читает)
		 * @param deltaTime  время кадра в секундах
		 */
		void update(const Input& input, float deltaTime) noexcept;

		// -------------------------------------------------------
		// Геттеры
		// -------------------------------------------------------
		[[nodiscard]] auto getPosition()   const noexcept -> const glm::vec3& { return m_position;  }
		[[nodiscard]] auto getVelocity()   const noexcept -> const glm::vec3& { return m_velocity;  }
		[[nodiscard]] auto isOnGround()    const noexcept -> bool              { return m_onGround;  }
		[[nodiscard]] auto isSprinting()   const noexcept -> bool              { return m_sprinting; }

		/** AABB в мировых координатах (текущий кадр). */
		[[nodiscard]] physics::AABB getAABB() const noexcept;

		/** Позиция глаз (для камеры и аудио-слушателя). */
		[[nodiscard]] glm::vec3 getEyePosition() const noexcept
		{
			return m_position + glm::vec3{ 0.0f, EYE_HEIGHT, 0.0f };
		}

		void setPosition(const glm::vec3& pos) noexcept { m_position = pos; }

	private:
		void processMovementInput(const Input& input, float deltaTime) noexcept;
		void processMouseInput   (const Input& input) noexcept;
		void applyPhysics        (float deltaTime) noexcept;

		physics::PhysicsWorld& m_physicsWorld;
		renderer::Camera&      m_camera;

		glm::vec3 m_position { 0.0f };
		glm::vec3 m_velocity { 0.0f };

		bool  m_onGround { false };
		bool  m_sprinting{ false };

		glm::dvec2 m_lastMousePos{ 0.0, 0.0 };
		bool       m_firstMouse  { true };
	};

} // namespace core::entities
