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

		void destroy(void);
		~Context(void) { this->destroy(); }

		Context(Context&& other) noexcept;
		Context& operator=(Context&& other) noexcept;

		[[nodiscard]] inline vk::Instance instance(void) const { return m_Instance; }
		[[nodiscard]] inline vk::DebugUtilsMessengerEXT dbg_messenger(void) const { return m_DebugMessenger; }
	private:
		vk::Instance m_Instance = nullptr;
		vk::DebugUtilsMessengerEXT m_DebugMessenger = nullptr;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_CONTEXT_HPP