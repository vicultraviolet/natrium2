#ifndef NA2_GRAPHICS_DEVICE_HPP
#define NA2_GRAPHICS_DEVICE_HPP

#include "Natrium2/Graphics/GraphicsContext.hpp"

namespace Na2::Graphics
{
	enum class DeviceExtension : u8 {
		None = 0,
		Swapchain
	};

	struct DeviceCreateInfo {
		View<const Context> context;

		std::set<DeviceExtension> extensions;

		// if no window is given, will create a temporary window if possible
		// a window surface is needed to properly select the queue families
		const Window& window = Window(1, 1, "TEMPORARY");
	};

	class Device {
	public:
		Device(void) = default;
		Device(DeviceCreateInfo&& info);

		void destroy(void);
		~Device(void) { this->destroy(); }

		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		inline void wait_all(void) const { return m_LogicalDevice.waitIdle(); }

		[[nodiscard]] inline View<const Context> context(void) const { return m_Context; }

		[[nodiscard]] inline vk::PhysicalDevice physical_device(void) const { return m_PhysicalDevice; }
		[[nodiscard]] inline vk::Device logical_device(void) const { return m_LogicalDevice; }

		[[nodiscard]] inline const auto& extensions(void) const { return m_Extensions; }

		[[nodiscard]] inline vk::Queue queue(u32 family)    const { return m_Queues[family]; }
		[[nodiscard]] inline vk::Queue graphics_queue(void) const { return m_Queues[m_GraphicsQueueFamily]; }
		[[nodiscard]] inline vk::Queue present_queue(void)  const { return m_Queues[m_PresentQueueFamily]; }
		[[nodiscard]] inline vk::Queue compute_queue(void)  const { return m_Queues[m_ComputeQueueFamily]; }

		[[nodiscard]] inline const auto& queues(void) const { return m_Queues; }

		[[nodiscard]] inline u32 graphics_queue_family(void) const { return m_GraphicsQueueFamily; }
		[[nodiscard]] inline u32 present_queue_family(void)  const { return m_PresentQueueFamily; }
		[[nodiscard]] inline u32 compute_queue_family(void)  const { return m_ComputeQueueFamily; }

		[[nodiscard]] inline const auto& unique_queue_families(void) const { return m_UniqueQueueFamilies; }

		[[nodiscard]] inline operator bool (void) const { return m_Context; }
	private:
		View<const Context> m_Context = nullptr;

		vk::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::Device m_LogicalDevice = nullptr;
		
		std::set<DeviceExtension> m_Extensions;

		u32 m_GraphicsQueueFamily = u32max,
			m_PresentQueueFamily  = u32max,
			m_ComputeQueueFamily  = u32max;
		Array<u32, 3> m_UniqueQueueFamilies;

		Array<vk::Queue, 3> m_Queues;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_DEVICE_HPP