#include "Pch.hpp"
#include "Natrium2/Core/Context.hpp"

#include "Natrium2/Core/Logger.hpp"

#ifdef NA2_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // NA_PLATFORM_WINDOWS

#ifdef NA2_USE_GLFW
	#include <GLFW/glfw3.h>
#endif // NA2_USE_GLFW

namespace Na2
{
	static std::filesystem::path getExecPath(void)
	{
#if defined(NA_PLATFORM_LINUX)
		return std::filesystem::canonical("/proc/self/exe");
#elif defined(NA_PLATFORM_WINDOWS)
		char exec_path_buffer[MAX_PATH];
		GetModuleFileNameA(nullptr, exec_path_buffer, MAX_PATH);
		return exec_path_buffer;
#else
		return "";
#endif // NA_PLATFORM
	}

	Context::Context(const ContextInitInfo& info)
		: m_ExecPath(getExecPath()),
		m_ExecDir(m_ExecPath.parent_path()),
		m_ExecName(m_ExecPath.filename()),
		m_Version("Pre-Alpha 2")
	{
		g_Logger.print_header();
		g_Logger.printf(Info, "Initializing Natrium version {}", m_Version);

	#ifdef NA2_USE_GLFW
		glfwSetErrorCallback([](int error, const char* description)
		{
			if (error == 65539)
				return;

			g_Logger.printf(Error, "GLFW Error#{}: {}", error, description);
			throw std::runtime_error(NA2_FORMAT("GLFW Error #{}", error));
		});

		int result = glfwInit();
		NA2_ASSERT(result, "Failed to initialize glfw!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	#endif // NA2_USE_GLFW
	}

	void Context::destroy(void)
	{
		g_Logger.print(Info, "Shutting down Natrium, Goodbye!");

	#ifdef NA2_USE_GLFW
		glfwTerminate();
	#endif // NA2_USE_GLFW

		if (Context::s_Context == this)
			s_Context = nullptr;
	}

	Context::Context(Context&& other) noexcept
	: m_ExecPath(std::move(other.m_ExecPath)),
	  m_ExecDir(std::move(other.m_ExecDir)),
	  m_ExecName(std::move(other.m_ExecName)),
	  m_Version(std::move(other.m_Version))
	{
		Context::s_Context = this;
	}

	Context& Context::operator=(Context&& other) noexcept
	{
		m_ExecPath = std::move(other.m_ExecPath);
		m_ExecDir = std::move(other.m_ExecDir);
		m_ExecName = std::move(other.m_ExecName);
		m_Version = std::move(other.m_Version);

		Context::s_Context = this;
		return *this;
	}
} // namespace Na
