#include "Pch.hpp"
#include "Natrium2/Core/Context.hpp"

#include "Natrium2/Core/Logger.hpp"

#ifdef NA2_PLATFORM_WINDOWS
	#include <windows.h>
	#include <mmsystem.h>
#endif // NA_PLATFORM_WINDOWS

#ifdef NA2_USE_GLFW
	#include <GLFW/glfw3.h>
#endif // NA2_USE_GLFW

#include "Natrium2/Core/EventQueue.hpp"

namespace Na2
{
#ifdef NA2_USE_GLFW
	namespace Platform::Desktop { extern ArrayList<GLFWgamepadstate> previousGamepadStates; }
#endif // NA2_USE_GLFW

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

	Context::Context(const ContextCreateInfo& info)
	: m_ExecPath(getExecPath()),
	  m_ExecDir(m_ExecPath.parent_path()),
	  m_ExecName(m_ExecPath.filename()),
	  m_Version("Pre-Alpha 2"),
	  m_EngineAssetsDirectory(info.engine_assets_dir),
	  m_ShaderOutputDirectory(info.shader_output_dir)
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

		glfwSetJoystickCallback([](int jid, int event)
		{
			if (!glfwJoystickIsGamepad(jid))
				return;

			if (event == GLFW_CONNECTED)
			{
				EventQueue::Get()->add(Event{.gamepad_connected = {
					EventType::GamepadConnected,
					false,
					nullptr,
					(u8)(jid + 1)
				}});
			} else
			if (event == GLFW_DISCONNECTED)
			{
				EventQueue::Get()->add(Event{.gamepad_disconnected = {
					EventType::GamepadDisconnected,
					false,
					nullptr,
					(u8)(jid + 1)
				}});
				Platform::Desktop::previousGamepadStates[jid] = {};
			}
		});
	#endif // NA2_USE_GLFW

		if (!std::filesystem::exists(m_EngineAssetsDirectory))
			std::filesystem::create_directories(m_EngineAssetsDirectory);

		if (!std::filesystem::exists(m_ShaderOutputDirectory))
			std::filesystem::create_directories(m_ShaderOutputDirectory);

	#ifdef NA2_PLATFORM_WINDOWS
		timeBeginPeriod(1);
	#endif // NA2_PLATFORM_WINDOWS
		
	}

	void Context::destroy(void)
	{
		g_Logger.print(Info, "Shutting down Natrium, Goodbye!");

	#ifdef NA2_PLATFORM_WINDOWS
		timeEndPeriod(1);
	#endif // NA2_PLATFORM_WINDOWS

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
	  m_Version(std::move(other.m_Version)),
	  m_EngineAssetsDirectory(std::move(other.m_EngineAssetsDirectory)),
	  m_ShaderOutputDirectory(std::move(other.m_ShaderOutputDirectory))
	{
		if (&other == Context::s_Context)
			Context::s_Context = this;
	}

	Context& Context::operator=(Context&& other) noexcept
	{
		if (this == &other)
			return *this;

		m_ExecPath = std::move(other.m_ExecPath);
		m_ExecDir = std::move(other.m_ExecDir);
		m_ExecName = std::move(other.m_ExecName);
		m_Version = std::move(other.m_Version);
		m_EngineAssetsDirectory = std::move(other.m_EngineAssetsDirectory);
		m_ShaderOutputDirectory = std::move(other.m_ShaderOutputDirectory);

		if (&other == Context::s_Context)
			Context::s_Context = this;

		return *this;
	}
} // namespace Na
