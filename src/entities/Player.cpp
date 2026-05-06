#include "Player.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace core::entities
{
	Player::Player(physics::PhysicsWorld& physicsWorld,
	               renderer::Camera&      camera,
	               glm::vec3              spawnPos) noexcept
		: m_physicsWorld(physicsWorld)
		, m_camera(camera)
		, m_position(spawnPos)
	{
		m_camera.setPosition(getEyePosition());
	}

	// -----------------------------------------------------------------------
	// Основной update
	// -----------------------------------------------------------------------

	void Player::update(const Input& input, float deltaTime) noexcept
	{
		processMouseInput(input);
		processMovementInput(input, deltaTime);
		applyPhysics(deltaTime);

		// Обновляем позицию камеры и аудио-слушателя
		m_camera.setPosition(getEyePosition());
	}

	// -----------------------------------------------------------------------
	// Мышь → поворот камеры
	// -----------------------------------------------------------------------

	void Player::processMouseInput(const Input& input) noexcept
	{
		const glm::dvec2 mousePos = input.getMousePosition();

		if (m_firstMouse)
		{
			m_lastMousePos = mousePos;
			m_firstMouse   = false;
			return;
		}

		const float dx = static_cast<float>(mousePos.x - m_lastMousePos.x);
		const float dy = static_cast<float>(m_lastMousePos.y - mousePos.y);

		m_lastMousePos = mousePos;

		m_camera.processMouseMovement(dx, dy);
	}

	// -----------------------------------------------------------------------
	// Клавиатура → горизонтальная скорость
	// -----------------------------------------------------------------------

	void Player::processMovementInput(const Input& input, float deltaTime) noexcept
	{
		m_sprinting = input.isKeyPressed(Key::LeftShift) && m_onGround;
		const float speed = m_sprinting ? SPRINT_SPEED : WALK_SPEED;

		glm::vec3 forward = m_camera.getForward();
		forward.y = 0.0f;
		if (glm::length(forward) > 0.001f)
			forward = glm::normalize(forward);

		const glm::vec3 right = glm::normalize(glm::cross(forward, { 0.0f, 1.0f, 0.0f }));

		glm::vec3 moveDir{ 0.0f };

		if (input.isKeyPressed(Key::W)) moveDir += forward;
		if (input.isKeyPressed(Key::S)) moveDir -= forward;
		if (input.isKeyPressed(Key::D)) moveDir += right;
		if (input.isKeyPressed(Key::A)) moveDir -= right;

		if (glm::length(moveDir) > 0.001f)
			moveDir = glm::normalize(moveDir);

		const float control = m_onGround ? 1.0f : AIR_CONTROL;

		m_velocity.x = moveDir.x * speed * control;
		m_velocity.z = moveDir.z * speed * control;

		if (input.isKeyJustPressed(Key::Space) && m_onGround)
		{
			m_velocity.y = JUMP_VELOCITY;
			m_onGround   = false;
		}
	}

	// -----------------------------------------------------------------------
	// Физика: гравитация + sweepMove
	// -----------------------------------------------------------------------

	void Player::applyPhysics(float deltaTime) noexcept
	{
		if (!m_onGround)
		{
			m_velocity.y += m_physicsWorld.getGravity() * deltaTime;

			// Ограничиваем скорость падения (terminal velocity)
			constexpr float TERMINAL_VELOCITY = -50.0f;
			m_velocity.y = std::max(m_velocity.y, TERMINAL_VELOCITY);
		}

		const glm::vec3 delta = m_velocity * deltaTime;

		const physics::AABB currentAABB = getAABB();
		bool onGround = false;
		const glm::vec3 resolved = m_physicsWorld.sweepMove(currentAABB, delta, onGround);

		m_onGround = onGround;
		m_position += resolved;

		if (std::abs(resolved.x) < std::abs(delta.x) * 0.99f) m_velocity.x = 0.0f;
		if (std::abs(resolved.z) < std::abs(delta.z) * 0.99f) m_velocity.z = 0.0f;

		if (m_onGround)
		{
			m_velocity.y = 0.0f;
		}
		else if (std::abs(resolved.y) < std::abs(delta.y) * 0.99f && m_velocity.y > 0.0f)
		{
			m_velocity.y = 0.0f;
		}
	}

	// -----------------------------------------------------------------------
	// Геттеры
	// -----------------------------------------------------------------------

	physics::AABB Player::getAABB() const noexcept
	{
		constexpr float halfWidth = PLAYER_WIDTH * 0.5f;
		return
		{
			m_position + glm::vec3{ -halfWidth, 0.0f,         -halfWidth },
			m_position + glm::vec3{  halfWidth, PLAYER_HEIGHT,  halfWidth }
		};
	}

} // namespace core::entities
