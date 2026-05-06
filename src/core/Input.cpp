#include "Input.h"

#include "Window.h"
#include <GLFW/glfw3.h>

namespace core
{
	void Input::init(Window& window)
	{
		m_windowHandle = window.getNativeHandle();

		glfwSetKeyCallback        (m_windowHandle, glfwKeyCallback);
		glfwSetMouseButtonCallback(m_windowHandle, glfwMouseButtonCallback);
		glfwSetCursorPosCallback  (m_windowHandle, glfwCursorPosCallback);

		glfwSetWindowUserPointer(m_windowHandle, this);
	}

	void Input::update() noexcept
	{
		m_previousKeys    = m_currentKeys;
		m_previousMousePos = m_mousePos;

		glfwGetCursorPos(m_windowHandle, &m_mousePos.x, &m_mousePos.y);

		m_mouseDelta.x = m_mousePos.x - m_previousMousePos.x;
		m_mouseDelta.y = m_mousePos.y - m_previousMousePos.y;
	}

	void Input::setCursorMode(CursorMode mode) noexcept
	{
		if (!m_windowHandle) return;

		switch (mode)
		{
		case CursorMode::Normal:
			glfwSetInputMode(m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case CursorMode::Hidden:
			glfwSetInputMode(m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
		case CursorMode::Disabled:
			glfwSetInputMode(m_windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_firstMouse = true;
			break;
		default:
			break;
		}
	}

	// === GLFW Колбэки ===
	void glfwKeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		if (key >= 0 && key < static_cast<int>(input->KEY_COUNT))
		{
			const bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
			input->m_currentKeys.set(static_cast<size_t>(key), pressed);
		}
	}

	void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
	{
		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		if (button >= 0 && button < static_cast<int>(input->KEY_COUNT))
		{
			const bool pressed = (action == GLFW_PRESS);
			input->m_currentKeys.set(static_cast<size_t>(button), pressed);
		}
	}

	void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		if (input->m_firstMouse)
		{
			input->m_mousePos         = { xpos, ypos };
			input->m_previousMousePos = input->m_mousePos;
			input->m_firstMouse       = false;
		}

		input->m_mousePos = { xpos, ypos };
	}

} // namespace core
