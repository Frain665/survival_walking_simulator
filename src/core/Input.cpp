#include "Input.h"

#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace core
{
	void Input::init(Window& window)
	{
		m_windowHandle = window.getNativeHandle();

		glfwSetWindowUserPointer  (m_windowHandle, this);
		glfwSetKeyCallback        (m_windowHandle, glfwKeyCallback);
		glfwSetMouseButtonCallback(m_windowHandle, glfwMouseButtonCallback);
		glfwSetCursorPosCallback  (m_windowHandle, glfwCursorPosCallback);
	}

	void Input::update() noexcept
	{
		m_justPressed.reset();
		m_justReleased.reset();

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

	void glfwKeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
	{
		std::cout << "KEY: " << key << " action: " << action << "\n";  // ВРЕМЕННО

		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		const auto idx = static_cast<size_t>(key);
		if (key < 0 || idx >= Input::KEY_COUNT) return;

		if (action == GLFW_PRESS)
		{
			input->m_currentKeys.set(idx);
			input->m_justPressed.set(idx);
		}
		else if (action == GLFW_RELEASE)
		{
			input->m_currentKeys.reset(idx);
			input->m_justReleased.set(idx);
		}
		
		if (key == 341) std::cout << "CTRL: action = " << action << "\n";
	}

	void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
	{
		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		const auto idx = static_cast<size_t>(button);
		if (button < 0 || idx >= Input::KEY_COUNT) return;

		if (action == GLFW_PRESS)
		{
			input->m_currentKeys.set(idx);
			input->m_justPressed.set(idx);
		}
		else if (action == GLFW_RELEASE)
		{
			input->m_currentKeys.reset(idx);
			input->m_justReleased.set(idx);
		}
	}

	void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
		if (!input) return;

		if (input->m_firstMouse)
		{
			input->m_mousePos         = { xpos, ypos };
			input->m_previousMousePos = { xpos, ypos };
			input->m_firstMouse       = false;
		}

		input->m_mousePos = { xpos, ypos };
	}

} // namespace core
