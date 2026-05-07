#include "Window.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

namespace core
{
	Window::Window(std::string title, int width, int height)
		: m_title(std::move(title))
		, m_width(width)
		, m_height(height)
	{
	}

	Window::~Window() noexcept
	{
		shutdown();
	}

	void Window::init()
	{
		if (m_windowHandle)
		{
			return;
		}

		if (glfwInit() == GLFW_FALSE)
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}
		m_glfwInitialized = true;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_windowHandle = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
		if (!m_windowHandle)
		{
			shutdown();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwMakeContextCurrent(m_windowHandle);
		glfwSwapInterval(1);
	}

	void Window::pollEvents() const noexcept
	{
		glfwPollEvents();
	}

	void Window::swapBuffers() const noexcept
	{
		if (m_windowHandle)
		{
			glfwSwapBuffers(m_windowHandle);
		}
	}

	void Window::requestClose() noexcept
	{
		if (m_windowHandle)
		{
			glfwSetWindowShouldClose(m_windowHandle, GLFW_TRUE);
		}
	}

	auto Window::shouldClose() const noexcept -> bool
	{
		return m_windowHandle ? glfwWindowShouldClose(m_windowHandle) != 0 : true;
	}

	auto Window::isValid() const noexcept -> bool
	{
		return m_windowHandle != nullptr;
	}

	auto Window::getNativeHandle() const noexcept -> GLFWwindow*
	{
		return m_windowHandle;
	}

	void Window::shutdown() noexcept
	{
		if (m_windowHandle)
		{
			glfwDestroyWindow(m_windowHandle);
			m_windowHandle = nullptr;
		}

		if (m_glfwInitialized)
		{
			glfwTerminate();
			m_glfwInitialized = false;
		}
	}

} // namespace core