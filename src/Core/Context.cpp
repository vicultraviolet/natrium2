#include "Pch.hpp"
#include "Natrium2/Core/Context.hpp"

#include "Natrium2/Core/Logger.hpp"

#if defined(NA2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif // NA_PLATFORM_WINDOWS

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
	}

	void Context::destroy(void)
	{
		g_Logger.print(Info, "Shutting down Natrium, Goodbye!");

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
