#pragma once

#include <bitset>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace core
{
	class Window;
	void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);

	enum class Key : int
	{
		Space   = 32,
		Escape  = 256,
		Enter   = 257,
		Tab     = 258,

		W = 87, A = 65, S = 83, D = 68,
		Q = 81, E = 69, R = 82, F = 70,

		_0 = 48, _1 = 49, _2 = 50, _3 = 51,
		_4 = 52, _5 = 53, _6 = 54, _7 = 55,
		_8 = 56, _9 = 57,

		LeftShift   = 340,
		LeftControl = 341,
		LeftAlt     = 342,

		MouseLeft   = 0,
		MouseRight  = 1, 
		MouseMiddle = 2,

		// Функциональные клавиши
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,

	};

	enum class CursorMode
	{
		Normal,
		Hidden,
		Disabled
	};

	class Input final
	{
	public:
		Input()  = default;
		~Input() = default;

		Input(const Input&) = delete;
		Input(Input&&) noexcept = default;

		auto operator=(const Input&)      -> Input& = delete;
		auto operator=(Input&&) noexcept  -> Input& = default;

		void init(Window& window);
		void update() noexcept;

		[[nodiscard]] auto isKeyPressed(Key key) const noexcept -> bool
		{
			const auto idx = static_cast<size_t>(key);
			return idx < m_currentKeys.size() && m_currentKeys.test(idx);
		}

		[[nodiscard]] auto isKeyJustPressed(Key key) const noexcept -> bool
		{
			const auto idx = static_cast<size_t>(key);
			return idx < m_currentKeys.size() &&
				m_currentKeys.test(idx) &&
				!m_previousKeys.test(idx);
		}

		[[nodiscard]] auto isKeyJustReleased(Key key) const noexcept -> bool
		{
			const auto idx = static_cast<size_t>(key);
			return idx < m_currentKeys.size() &&
				!m_currentKeys.test(idx) &&
				m_previousKeys.test(idx);
		}

		// === Состояние мыши ===

		[[nodiscard]] auto getMousePosition() const noexcept -> glm::dvec2 { return m_mousePos;   }
		[[nodiscard]] auto getMouseDelta()    const noexcept -> glm::dvec2 { return m_mouseDelta;  }

		void setCursorMode(CursorMode mode) noexcept;

		friend void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		friend void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		friend void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);

	private:
		static constexpr size_t KEY_COUNT = 512;

		std::bitset<KEY_COUNT> m_currentKeys;
		std::bitset<KEY_COUNT> m_previousKeys;

		glm::dvec2 m_mousePos        { 0.0, 0.0 };
		glm::dvec2 m_previousMousePos{ 0.0, 0.0 };
		glm::dvec2 m_mouseDelta      { 0.0, 0.0 };

		GLFWwindow* m_windowHandle{ nullptr };
		bool m_firstMouse{ true };
	};

} // namespace core
