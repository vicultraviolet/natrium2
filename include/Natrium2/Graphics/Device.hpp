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

		[[nodiscard]] inline vk::PhysicalDevice physical_device(void) const { return m_PhysicalDevice; }
		[[nodiscard]] inline vk::Device logical_device(void) const { return m_LogicalDevice; }

		[[nodiscard]] inline const auto& extensions(void) const { return m_Extensions; }

		[[nodiscard]] inline vk::Queue graphics_queue(void) const { return m_LogicalDevice.getQueue(m_GraphicsQueueFamily, 0); }
		[[nodiscard]] inline vk::Queue present_queue(void)  const { return m_LogicalDevice.getQueue(m_PresentQueueFamily,  0); }
		[[nodiscard]] inline vk::Queue compute_queue(void)  const { return m_LogicalDevice.getQueue(m_ComputeQueueFamily,  0); }
		[[nodiscard]] inline vk::Queue transfer_queue(void) const { return m_LogicalDevice.getQueue(m_TransferQueueFamily, 0); }

		[[nodiscard]] inline u32 graphics_queue_family(void) const { return m_GraphicsQueueFamily; }
		[[nodiscard]] inline u32 present_queue_family(void)  const { return m_PresentQueueFamily; }
		[[nodiscard]] inline u32 compute_queue_family(void)  const { return m_ComputeQueueFamily; }
		[[nodiscard]] inline u32 transfer_queue_family(void) const { return m_TransferQueueFamily; }
	private:
		vk::raii::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::raii::Device m_LogicalDevice = nullptr;
		
		std::set<DeviceExtension> m_Extensions;

		u32 m_GraphicsQueueFamily = u32max,
			m_PresentQueueFamily  = u32max,
			m_ComputeQueueFamily  = u32max,
			m_TransferQueueFamily = u32max;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_DEVICE_HPP