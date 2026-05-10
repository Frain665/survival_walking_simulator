#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core::renderer
{
	class Camera
	{
	public:
		enum class MoveDirection { Forward, Backward, Left, Right };

		explicit Camera(
			glm::vec3 position = { 0.0f, 1.75f, 0.0f },
			float     yaw = -90.0f,
			float     pitch = 0.0f,
			float     fovDeg = 70.0f,
			float     nearPlane = 0.05f,
			float     farPlane = 1000.0f
		) noexcept;

		[[nodiscard]] auto getViewMatrix()                        const noexcept -> glm::mat4;
		[[nodiscard]] auto getProjectionMatrix(float aspectRatio) const noexcept -> glm::mat4;

		void processMouseMovement(float xOffset, float yOffset,
			bool constrainPitch = true) noexcept;

		void update(float deltaTime, float speedFactor, bool isMoving) noexcept;

		void setPosition(const glm::vec3& position) noexcept { m_position = position; }
		void setFov(float fovDeg)                   noexcept;
		void setSensitivity(float sensitivity)      noexcept { m_sensitivity = sensitivity; }

		[[nodiscard]] auto getPosition() const noexcept -> const glm::vec3& { return m_position; }
		[[nodiscard]] auto getForward()  const noexcept -> const glm::vec3& { return m_front; }
		[[nodiscard]] auto getRight()    const noexcept -> const glm::vec3& { return m_right; }
		[[nodiscard]] auto getUp()       const noexcept -> const glm::vec3& { return m_up; }
		[[nodiscard]] auto getYaw()      const noexcept -> float { return m_yaw; }
		[[nodiscard]] auto getPitch()    const noexcept -> float { return m_pitch; }
		[[nodiscard]] auto getFov()      const noexcept -> float { return m_fov; }

	private:
		void updateVectors() noexcept;

		glm::vec3 m_position;
		glm::vec3 m_front{ 0.0f, 0.0f, -1.0f };
		glm::vec3 m_up{ 0.0f, 1.0f,  0.0f };
		glm::vec3 m_right{ 1.0f, 0.0f,  0.0f };
		glm::vec3 m_worldUp{ 0.0f, 1.0f,  0.0f };

		float m_yaw;
		float m_pitch;
		float m_fov;
		float m_near;
		float m_far;

		float m_sensitivity{ 0.1f };

		// Сглаживание мыши
		float m_smoothYaw{ 0.0f };
		float m_smoothPitch{ 0.0f };
		static constexpr float MOUSE_SMOOTH{ 20.0f };

		// Дыхание камеры
		float m_breathTime{ 0.0f };
		float m_breathAlpha{ 0.0f };
	};

} // namespace core::renderer