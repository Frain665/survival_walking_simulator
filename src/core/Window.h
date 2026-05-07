#pragma once

#include <string>

struct GLFWwindow;

namespace core
{
	class Window final
	{
	public:
		Window(std::string title, int width, int height);
		~Window() noexcept;

		Window(const Window&) = delete;
		Window(Window&&) = delete;
		auto operator=(const Window&) -> Window& = delete;
		auto operator=(Window&&) -> Window& = delete;

		void init();
		void pollEvents() const noexcept;
		void swapBuffers() const noexcept;
		void requestClose() noexcept;

		[[nodiscard]] auto shouldClose() const noexcept -> bool;
		[[nodiscard]] auto isValid() const noexcept -> bool;
		[[nodiscard]] auto getNativeHandle() const noexcept -> GLFWwindow*;

		[[nodiscard]] auto getWidth() const noexcept -> int { return m_width; }
		[[nodiscard]] auto getHeight() const noexcept -> int { return m_height; }

	private:
		void shutdown() noexcept;

		std::string m_title;
		int m_width{ 1280 };
		int m_height{ 720 };

		GLFWwindow* m_windowHandle{ nullptr };
		bool m_glfwInitialized{ false };
	};

} // namespace core