#ifndef NA2_GRAPHICS_CONTEXT_HPP
#define NA2_GRAPHICS_CONTEXT_HPP

#include "Natrium2/Core.hpp"
#include "Natrium2/Core/Logger.hpp"

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
		Context(const ContextCreateInfo& createInfo);

	private:
		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
		vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
	};

	inline Logger<> g_Logger{ "Vulkan" };
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_CONTEXT_HPP