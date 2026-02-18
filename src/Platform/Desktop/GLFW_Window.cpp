#include "Pch.hpp"
#include "Natrium2/Platform/Desktop/GLFW_Window.hpp"

#ifdef NA2_USE_GLFW

#include <GLFW/glfw3.h>
#include "Natrium2/Core/EventQueue.hpp"
#include "Natrium2/Platform/Desktop/GLFW_InputConstants.hpp"

namespace Na2::Platform::Desktop
{
	using Window = GLFW_Window;

	Window::GLFW_Window(u32 width, u32 height, const std::string_view& title)
	: m_Width(width), m_Height(height)
	{
		m_Window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
		NA2_ASSERT(m_Window, "Failed to create glfw window!");

		glfwSetWindowUserPointer(m_Window, this);

		Window_SetGLFWCallbacks(m_Window);

		m_Focused = glfwGetWindowAttrib(m_Window, GLFW_FOCUSED) != 0;
	}

	void Window::destroy(void)
	{
		if (!m_Window)
			return;

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}

	void Window::focus(void)
	{
		glfwFocusWindow(m_Window);
		m_Focused = true;
	}

	void Window::unfocus(void)
	{
		glfwFocusWindow(nullptr);
		m_Focused = false;
	}

	void Window::capture_mouse(void)
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		m_MouseCaptured = true;
	}

	void Window::release_mouse(void)
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		m_MouseCaptured = false;
	}

	void Window::minimize(void)
	{
		glfwIconifyWindow(m_Window);
		m_Minimized = true;
	}

	void Window::restore(void)
	{
		glfwRestoreWindow(m_Window);
		m_Minimized = false;
	}

	void Window::set_size(u32 width, u32 height)
	{
		glfwSetWindowSize(m_Window, m_Width = width, m_Height = height);
	}

	void Window::set_title(const std::string_view& title)
	{
		glfwSetWindowTitle(m_Window, title.data());
	}

	std::string_view Window::title(void) const
	{
		return glfwGetWindowTitle(m_Window);
	}

	Window::GLFW_Window(Window&& other) noexcept
	: m_Window(std::exchange(other.m_Window, nullptr)),
	  m_Width(std::exchange(other.m_Width, 0)),
	  m_Height(std::exchange(other.m_Height, 0)),
	  m_Focused(std::exchange(other.m_Focused, false)),
	  m_Minimized(std::exchange(other.m_Minimized, false)),
	  m_MouseCaptured(std::exchange(other.m_MouseCaptured, false))
	{
		glfwSetWindowUserPointer(m_Window, this);
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (m_Window == other.m_Window)
			return *this;

		if (m_Window)
			glfwDestroyWindow(m_Window);

		m_Window = std::exchange(other.m_Window, nullptr);
		m_Width = std::exchange(other.m_Width, 0);
		m_Height = std::exchange(other.m_Height, 0);
		m_Focused = std::exchange(other.m_Focused, false);
		m_Minimized = std::exchange(other.m_Minimized, false);
		m_MouseCaptured = std::exchange(other.m_MouseCaptured, false);

		glfwSetWindowUserPointer(m_Window, this);

		return *this;
	}

	void Window_SetGLFWCallbacks(GLFWwindow* _window)
	{
		glfwSetKeyCallback(_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
				EventQueue::Get()->add(Event{.key_pressed = {
					EventType::KeyPressed,
					false,
					__window,
					KeyFromGLFW(key),
					KeyModFromGLFW(mods),
					false
				}});
				break;
			case GLFW_REPEAT:
				EventQueue::Get()->add(Event{.key_pressed = {
					EventType::KeyPressed,
					false,
					__window,
					KeyFromGLFW(key),
					KeyModFromGLFW(mods),
					true
				}});
				break;
			case GLFW_RELEASE:
				EventQueue::Get()->add(Event{.key_released = {
					EventType::KeyReleased,
					false,
					__window,
					KeyFromGLFW(key),
					KeyModFromGLFW(mods)
				}});
				break;
			}
		});
		glfwSetCharCallback(_window, [](GLFWwindow* window, unsigned int codepoint)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			EventQueue::Get()->add(Event{.char_input = {
				EventType::CharInput,
				false,
				__window,
				codepoint
			}});
		});
		glfwSetWindowSizeCallback(_window, [](GLFWwindow* window, int width, int height)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			__window->m_Width = width;
			__window->m_Height = height;

			EventQueue::Get()->add(Event{.window_resized = {
				EventType::WindowResized,
				false,
				__window,
				(u32)width, (u32)height
			}});
		});
		glfwSetWindowCloseCallback(_window, [](GLFWwindow* window)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			EventQueue::Get()->add(Event{.window_closed = {
				EventType::WindowClosed,
				false,
				__window
			}});
		});
		glfwSetWindowFocusCallback(_window, [](GLFWwindow* window, int focus)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			__window->m_Focused = focus;

			if (focus)
				EventQueue::Get()->add(Event{.window_focused = {
					EventType::WindowFocused,
					false,
					__window
				}});
			else
				EventQueue::Get()->add(Event{.window_lost_focus = {
					EventType::WindowLostFocus,
					false,
					__window
				}});
		});
		glfwSetWindowIconifyCallback(_window, [](GLFWwindow* window, int iconified)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);

			__window->m_Minimized = iconified;

			if (iconified)
				EventQueue::Get()->add(Event{.window_minimized = {
					EventType::WindowMinimized,
					false,
					__window
				}});
			else
				EventQueue::Get()->add(Event{.window_restored = {
					EventType::WindowRestored,
					false,
					__window
				}});
		});
		glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double x, double y)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);
			EventQueue::Get()->add(Event{.mouse_moved = {
				EventType::MouseMoved,
				false,
				__window,
				(f32)x, (f32)y
			}});
		});
		glfwSetScrollCallback(_window, [](GLFWwindow* window, double x_offset, double y_offset)
		{
			auto __window = (Window*)glfwGetWindowUserPointer(window);
			EventQueue::Get()->add(Event{.mouse_scrolled = {
				EventType::MouseScrolled,
				false,
				__window,
				(f32)x_offset, (f32)y_offset
			}});
		});
		glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods)
		{
			Window* __window = (Window*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
				EventQueue::Get()->add(Event{.mouse_button_pressed = {
					EventType::MouseButtonPressed,
					false,
					__window,
					MouseButtonFromGLFW(button)
				}});
				break;
			case GLFW_RELEASE:
				EventQueue::Get()->add(Event{.mouse_button_released = {
					EventType::MouseButtonReleased,
					false,
					__window,
					MouseButtonFromGLFW(button)
				}});
				break;
			}
		});
	}
} // namespace Na2::Platform::Desktop

#endif // NA2_USE_GLFW
