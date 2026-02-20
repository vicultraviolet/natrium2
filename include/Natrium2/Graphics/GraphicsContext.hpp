#ifndef NA2_GRAPHICS_CONTEXT_HPP
#define NA2_GRAPHICS_CONTEXT_HPP

#include "Natrium2/Core.hpp"
#include "Natrium2/Core/Logger.hpp"

#include "Natrium2/Core/Window.hpp"

#ifndef NA2_CONFIG_DIST
#define NA2_VK_VALIDATION_LAYERS
#endif // NA2_CONFIG_DIST

namespace Na2::Graphics
{
	enum class DeviceExtension : u8 {
		None = 0,
		Swapchain
	};
	using DeviceExtensions = std::set<DeviceExtension>;

	struct ContextCreateInfo {
		DeviceExtensions extensions;
	};

	class Context {
	public:
		Context(void) = default;
		Context(ContextCreateInfo&& info);

		[[nodiscard]] static inline bool Exists(void) { return Context::s_Instance; }
		[[nodiscard]] static inline View<Context> Get(void) { return Context::s_Instance; }

		inline void bind(void) { Context::s_Instance = this; }
		inline void unbind(void) { Context::s_Instance = nullptr; }

		[[nodiscard]] inline const auto& device_extensions(void) { return m_DeviceExtensions; }
	private:
		DeviceExtensions m_DeviceExtensions;

		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
		vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;

		ArrayList<const char*> m_VkDeviceExtensions{ 4 };

		static inline View<Context> s_Instance = nullptr;
	};

	inline Logger<> g_Logger{ "Vulkan" };
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_CONTEXT_HPP