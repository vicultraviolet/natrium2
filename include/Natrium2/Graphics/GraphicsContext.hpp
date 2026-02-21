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
	struct ContextCreateInfo {

	};

	class Context {
		friend class Device;
	public:
		Context(void) = default;
		Context(ContextCreateInfo&& info);
	private:
		vk::raii::Context m_Context;
		vk::raii::Instance m_Instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_CONTEXT_HPP