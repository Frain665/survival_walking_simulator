#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace core::renderer
{
	Camera::Camera(glm::vec3 position, float yaw, float pitch,
		float fovDeg, float nearPlane, float farPlane) noexcept
		: m_position(position)
		, m_yaw(yaw)
		, m_pitch(pitch)
		, m_smoothYaw(yaw)
		, m_smoothPitch(pitch)
		, m_fov(fovDeg)
		, m_near(nearPlane)
		, m_far(farPlane)
	{
		updateVectors();
	}

	auto Camera::getViewMatrix() const noexcept -> glm::mat4
	{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}

	auto Camera::getProjectionMatrix(float aspectRatio) const noexcept -> glm::mat4
	{
		return glm::perspective(glm::radians(m_fov), aspectRatio, m_near, m_far);
	}

	void Camera::processMouseMovement(float xOffset, float yOffset,
		bool constrainPitch) noexcept
	{
		m_yaw += xOffset * m_sensitivity;
		m_pitch += yOffset * m_sensitivity;

		if (constrainPitch)
			m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

		if (m_yaw > 360.0f) m_yaw -= 360.0f;
		if (m_yaw < -360.0f) m_yaw += 360.0f;
	}

	void Camera::update(float deltaTime, float speedFactor, bool isMoving) noexcept
	{
		// Сглаживание поворотов мышью через lerp
		const float t = std::min(1.0f, MOUSE_SMOOTH * deltaTime);
		m_smoothYaw += (m_yaw - m_smoothYaw) * t;
		m_smoothPitch += (m_pitch - m_smoothPitch) * t;

		// Дыхание: плавно включается при простое, выключается при движении
		const float breathTarget = isMoving ? 0.0f : 1.0f;
		m_breathAlpha += (breathTarget - m_breathAlpha) * std::min(1.0f, 2.0f * deltaTime);

		if (m_breathAlpha > 0.001f)
		{
			m_breathTime += deltaTime;

			// Два синуса с разными частотами — выглядит органично
			const float breathX = std::sin(m_breathTime * 0.8f) * 0.0012f;
			const float breathY = std::sin(m_breathTime * 1.4f) * 0.0008f;

			// FOV чуть пульсирует — едва заметно
			const float breathFov = std::sin(m_breathTime * 0.6f) * 0.15f * m_breathAlpha;

			const float displayYaw = m_smoothYaw + glm::degrees(breathX) * m_breathAlpha;
			const float displayPitch = m_smoothPitch + glm::degrees(breathY) * m_breathAlpha;
			const float displayFov = m_fov + breathFov;

			const float yawRad = glm::radians(displayYaw);
			const float pitchRad = glm::radians(displayPitch);

			glm::vec3 front;
			front.x = std::cos(yawRad) * std::cos(pitchRad);
			front.y = std::sin(pitchRad);
			front.z = std::sin(yawRad) * std::cos(pitchRad);
			m_front = glm::normalize(front);
			m_right = glm::normalize(glm::cross(m_front, m_worldUp));
			m_up = glm::normalize(glm::cross(m_right, m_front));

			// Обновляем FOV временно через perspective — храним отдельно
			// TODO: вынести displayFov в getProjectionMatrix если нужен живой FOV
			(void)displayFov;
		}
		else
		{
			updateVectors();
		}
	}

	void Camera::setFov(float fovDeg) noexcept
	{
		m_fov = std::clamp(fovDeg, 1.0f, 120.0f);
	}

	void Camera::updateVectors() noexcept
	{
		const float yawRad = glm::radians(m_smoothYaw);
		const float pitchRad = glm::radians(m_smoothPitch);

		glm::vec3 front;
		front.x = std::cos(yawRad) * std::cos(pitchRad);
		front.y = std::sin(pitchRad);
		front.z = std::sin(yawRad) * std::cos(pitchRad);
		m_front = glm::normalize(front);

		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

} // namespace core::renderer