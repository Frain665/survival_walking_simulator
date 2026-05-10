#include "Player.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace core::entities
{
	Player::Player(physics::PhysicsWorld& physicsWorld,
		renderer::Camera& camera,
		glm::vec3              spawnPos) noexcept
		: m_physicsWorld(physicsWorld)
		, m_camera(camera)
		, m_position(spawnPos)
	{
		m_camera.setPosition(getEyePosition());
	}

	void Player::update(const Input& input, float deltaTime) noexcept
	{
		processMouseInput(input);
		updateCrouch(deltaTime);
		processMovementInput(input, deltaTime);
		applyPhysics(deltaTime);

		m_camera.setPosition(getEyePosition());

		const float horizSpeed = std::sqrt(m_velocity.x * m_velocity.x +
			m_velocity.z * m_velocity.z);
		const bool isMoving = horizSpeed > 0.5f && m_onGround;
		const float speedFactor = m_sprinting ? 1.0f : (m_crouching ? 0.3f : 0.7f);

		m_camera.update(deltaTime, speedFactor, isMoving);
	}

	void Player::processMouseInput(const Input& input) noexcept
	{
		const glm::dvec2 mousePos = input.getMousePosition();

		if (m_firstMouse)
		{
			m_lastMousePos = mousePos;
			m_firstMouse = false;
			return;
		}

		const float dx = static_cast<float>(mousePos.x - m_lastMousePos.x);
		const float dy = static_cast<float>(m_lastMousePos.y - mousePos.y);

		m_lastMousePos = mousePos;

		m_camera.processMouseMovement(dx, dy);
	}

	void Player::updateCrouch(float deltaTime) noexcept
	{
		const float targetHeight = m_crouching ? CROUCH_HEIGHT : PLAYER_HEIGHT;
		const float targetEyeHeight = m_crouching ? CROUCH_EYE_HEIGHT : EYE_HEIGHT;

		const float t = CROUCH_TRANSITION_SPEED * deltaTime;

		m_currentHeight += (targetHeight - m_currentHeight) * t;
		m_currentEyeHeight += (targetEyeHeight - m_currentEyeHeight) * t;

		if (std::abs(m_currentHeight - targetHeight) < 0.001f) m_currentHeight = targetHeight;
		if (std::abs(m_currentEyeHeight - targetEyeHeight) < 0.001f) m_currentEyeHeight = targetEyeHeight;
	}

	void Player::updateSprintSpeed(float deltaTime, float targetSpeed) noexcept
	{
		if (m_currentSpeed < targetSpeed)
		{
			m_currentSpeed += SPRINT_ACCEL * deltaTime;
			if (m_currentSpeed > targetSpeed) m_currentSpeed = targetSpeed;
		}
		else if (m_currentSpeed > targetSpeed)
		{
			m_currentSpeed -= SPRINT_DECEL * deltaTime;
			if (m_currentSpeed < targetSpeed) m_currentSpeed = targetSpeed;
		}
	}

	void Player::processMovementInput(const Input& input, float deltaTime) noexcept
	{
		const bool wantCrouch = input.isKeyPressed(Key::LeftControl);

		if (wantCrouch)
		{
			m_crouching = true;
			m_sprinting = false;
		}
		else if (m_crouching && canUncrouch())
		{
			m_crouching = false;
		}

		if (!m_crouching)
			m_sprinting = input.isKeyPressed(Key::LeftShift) && m_onGround;

		float targetSpeed = WALK_SPEED;
		if (m_crouching)      targetSpeed = CROUCH_SPEED;
		else if (m_sprinting) targetSpeed = SPRINT_SPEED;

		updateSprintSpeed(deltaTime, targetSpeed);

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

		m_velocity.x = moveDir.x * m_currentSpeed * control;
		m_velocity.z = moveDir.z * m_currentSpeed * control;

		if (input.isKeyJustPressed(Key::Space) && m_onGround && !m_crouching)
		{
			m_velocity.y = JUMP_VELOCITY;
			m_onGround = false;
		}
	}

	void Player::applyPhysics(float deltaTime) noexcept
	{
		if (!m_onGround)
		{
			m_velocity.y += m_physicsWorld.getGravity() * deltaTime;

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

	bool Player::canUncrouch() const noexcept
	{
		constexpr float halfWidth = PLAYER_WIDTH * 0.5f;

		const physics::AABB standingAABB =
		{
			m_position + glm::vec3{ -halfWidth, 0.0f,          -halfWidth },
			m_position + glm::vec3{  halfWidth, PLAYER_HEIGHT,  halfWidth }
		};

		return !m_physicsWorld.overlapsAny(standingAABB);
	}

	physics::AABB Player::getAABB() const noexcept
	{
		constexpr float halfWidth = PLAYER_WIDTH * 0.5f;
		return
		{
			m_position + glm::vec3{ -halfWidth, 0.0f,             -halfWidth },
			m_position + glm::vec3{  halfWidth, m_currentHeight,   halfWidth }
		};
	}

} // namespace core::entities