#pragma once

#include <glm/glm.hpp>

#include "physics/PhysicsWorld.h"
#include "renderer/Camera.h"
#include "core/Input.h"

namespace core::entities
{
	class Player
	{
	public:
		static constexpr float PLAYER_WIDTH = 0.6f;
		static constexpr float PLAYER_HEIGHT = 1.8f;
		static constexpr float CROUCH_HEIGHT = 1.2f;
		static constexpr float EYE_HEIGHT = 1.62f;
		static constexpr float CROUCH_EYE_HEIGHT = 1.05f;

		static constexpr float WALK_SPEED = 4.3f;
		static constexpr float SPRINT_SPEED = 5.6f;
		static constexpr float CROUCH_SPEED = 1.3f;
		static constexpr float JUMP_VELOCITY = 8.4f;
		static constexpr float AIR_CONTROL = 0.2f;

		static constexpr float CROUCH_TRANSITION_SPEED = 10.0f;
		static constexpr float SPRINT_ACCEL = 8.0f;
		static constexpr float SPRINT_DECEL = 12.0f;

		explicit Player(physics::PhysicsWorld& physicsWorld,
			renderer::Camera& camera,
			glm::vec3              spawnPos = { 0.0f, 64.0f, 0.0f }) noexcept;

		void update(const Input& input, float deltaTime) noexcept;

		[[nodiscard]] auto getPosition()  const noexcept -> const glm::vec3& { return m_position; }
		[[nodiscard]] auto getVelocity()  const noexcept -> const glm::vec3& { return m_velocity; }
		[[nodiscard]] auto isOnGround()   const noexcept -> bool { return m_onGround; }
		[[nodiscard]] auto isSprinting()  const noexcept -> bool { return m_sprinting; }
		[[nodiscard]] auto isCrouching()  const noexcept -> bool { return m_crouching; }

		[[nodiscard]] physics::AABB getAABB() const noexcept;

		[[nodiscard]] glm::vec3 getEyePosition() const noexcept
		{
			return m_position + glm::vec3{ 0.0f, m_currentEyeHeight, 0.0f };
		}

		void setPosition(const glm::vec3& pos) noexcept { m_position = pos; }

	private:
		void processMovementInput(const Input& input, float deltaTime) noexcept;
		void processMouseInput(const Input& input) noexcept;
		void applyPhysics(float deltaTime) noexcept;
		void updateCrouch(float deltaTime) noexcept;
		void updateSprintSpeed(float deltaTime, float targetSpeed) noexcept;

		bool canUncrouch() const noexcept;

		physics::PhysicsWorld& m_physicsWorld;
		renderer::Camera& m_camera;

		glm::vec3 m_position{ 0.0f };
		glm::vec3 m_velocity{ 0.0f };

		bool  m_onGround{ false };
		bool  m_sprinting{ false };
		bool  m_crouching{ false };

		float m_currentSpeed{ WALK_SPEED };
		float m_currentHeight{ PLAYER_HEIGHT };
		float m_currentEyeHeight{ EYE_HEIGHT };

		glm::dvec2 m_lastMousePos{ 0.0, 0.0 };
		bool       m_firstMouse{ true };
	};

} // namespace core::entities