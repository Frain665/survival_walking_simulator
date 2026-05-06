#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace core::renderer
{
	Camera::Camera(glm::vec3 position, float yaw, float pitch,
	               float fovDeg, float nearPlane, float farPlane) noexcept
		: m_position(position)
		, m_yaw(yaw)
		, m_pitch(pitch)
		, m_fov(fovDeg)
		, m_near(nearPlane)
		, m_far(farPlane)
	{
		updateVectors();
	}

	// -----------------------------------------------------------------------
	// Матрицы
	// -----------------------------------------------------------------------

	auto Camera::getViewMatrix() const noexcept -> glm::mat4
	{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}

	auto Camera::getProjectionMatrix(float aspectRatio) const noexcept -> glm::mat4
	{
		return glm::perspective(glm::radians(m_fov), aspectRatio, m_near, m_far);
	}

	// -----------------------------------------------------------------------
	// Ввод
	// -----------------------------------------------------------------------

	void Camera::processMouseMovement(float xOffset, float yOffset,
	                                   bool constrainPitch) noexcept
	{
		m_yaw   += xOffset * m_sensitivity;
		m_pitch += yOffset * m_sensitivity;

		if (constrainPitch)
		{
			m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
		}

		if (m_yaw >  360.0f) m_yaw -= 360.0f;
		if (m_yaw < -360.0f) m_yaw += 360.0f;

		updateVectors();
	}

	// -----------------------------------------------------------------------
	// Параметры
	// -----------------------------------------------------------------------

	void Camera::setFov(float fovDeg) noexcept
	{
		m_fov = std::clamp(fovDeg, 1.0f, 120.0f);
	}

	// -----------------------------------------------------------------------
	// Приватные
	// -----------------------------------------------------------------------

	void Camera::updateVectors() noexcept
	{
		const float yawRad   = glm::radians(m_yaw);
		const float pitchRad = glm::radians(m_pitch);

		glm::vec3 front;
		front.x = std::cos(yawRad) * std::cos(pitchRad);
		front.y = std::sin(pitchRad);
		front.z = std::sin(yawRad) * std::cos(pitchRad);
		m_front = glm::normalize(front);

		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up    = glm::normalize(glm::cross(m_right, m_front));
	}

} // namespace core::renderer
