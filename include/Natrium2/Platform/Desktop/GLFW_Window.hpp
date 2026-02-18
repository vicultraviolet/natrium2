#ifndef NA2_GLFW_WINDOW_HPP
#define NA2_GLFW_WINDOW_HPP

struct GLFWwindow;

namespace Na2::Platform::Desktop
{
	void Window_SetGLFWCallbacks(GLFWwindow* window);

	class GLFW_Window {
	public:
		GLFW_Window(void) = default;
		GLFW_Window(u32 width, u32 height, const std::string_view& title);

		~GLFW_Window(void) { this->destroy(); }
		void destroy(void);

		GLFW_Window(const GLFW_Window& other) = delete;
		GLFW_Window& operator=(const GLFW_Window& other) = delete;

		GLFW_Window(GLFW_Window&& other) noexcept;
		GLFW_Window& operator=(GLFW_Window&& other) noexcept;

		void focus(void);
		void unfocus(void);
		[[nodiscard]] inline bool focused(void) const { return m_Focused; }

		void capture_mouse(void);
		void release_mouse(void);
		[[nodiscard]] inline bool mouse_captured(void) const { return m_MouseCaptured; }

		void minimize(void);
		void restore(void);
		[[nodiscard]] inline bool minimized(void) const { return m_Minimized; }

		[[nodiscard]] inline u32 width(void) const { return m_Width; }
		[[nodiscard]] inline u32 height(void) const { return m_Height; }
		void set_size(u32 width, u32 height);

		void set_title(const std::string_view& title);
		[[nodiscard]] std::string_view title(void) const;

		[[nodiscard]] inline GLFWwindow* native(void) const { return m_Window; }
		inline operator bool(void) const { return m_Window; }
	private:
		friend void Window_SetGLFWCallbacks(GLFWwindow* window);
	private:
		GLFWwindow* m_Window = nullptr;
		u32 m_Width, m_Height;

		bool m_Focused = false;
		bool m_Minimized = false;
		bool m_MouseCaptured = false;
	};
} // namespace Na2::Platform::Desktop

#endif // NA2_GLFW_WINDOW_HPP