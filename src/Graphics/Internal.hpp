#ifndef NA2_GRAPHICS_INTERNAL_HPP
#define NA2_GRAPHICS_INTERNAL_HPP

#include "Natrium2/Core.hpp"
#include "Natrium2/Core/Window.hpp"

namespace Na2::Graphics
{
	[[nodiscard]] vk::raii::SurfaceKHR CreateSurface(
		const vk::raii::Instance& instance,
		const Window& window
	);

	struct QueueFamilyFindInfo {
		vk::raii::PhysicalDevice device;
		vk::QueueFlagBits desired_flags = (vk::QueueFlagBits)0U;
		vk::QueueFlags undesired_flags = (vk::QueueFlagBits)u32max;

		bool present = false;
		vk::SurfaceKHR surface = nullptr;
	};

	[[nodiscard]] u32 FindFirstQueueFamily(QueueFamilyFindInfo&&);
	[[nodiscard]] u32 FindBestQueueFamily(QueueFamilyFindInfo&&);

	class SurfaceDeviceInfo {
	public:
		SurfaceDeviceInfo(void) = default;
		SurfaceDeviceInfo(vk::PhysicalDevice, vk::SurfaceKHR);

		[[nodiscard]] inline vk::SurfaceCapabilitiesKHR capabilities(void) const { return m_Capabilities; }
		[[nodiscard]] inline const ArrayList<vk::SurfaceFormatKHR>& formats(void) const { return m_Formats; }
		[[nodiscard]] inline const ArrayList<vk::PresentModeKHR>& present_modes(void) { return m_PresentModes; }

		[[nodiscard]] inline operator bool(void) const { return m_Formats.size() && m_PresentModes.size(); }
	private:
		vk::SurfaceCapabilitiesKHR m_Capabilities;
		ArrayList<vk::SurfaceFormatKHR> m_Formats;
		ArrayList<vk::PresentModeKHR> m_PresentModes;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_INTERNAL_HPP