#ifndef NA2_GRAPHICS_INTERNAL_HPP
#define NA2_GRAPHICS_INTERNAL_HPP

#include "Natrium2/Core.hpp"
#include "Natrium2/Core/Window.hpp"

namespace Na2::Graphics
{
	[[nodiscard]] vk::raii::SurfaceKHR CreateSurface(
		const vk::raii::Instance& instance,
		Window& window
	);

	class QueueFamilyIndices {
	public:
		QueueFamilyIndices(void) = default;
		QueueFamilyIndices(vk::raii::PhysicalDevice, const vk::raii::SurfaceKHR&);

		[[nodiscard]] inline u32 graphics(void) const { return m_Graphics; }
		[[nodiscard]] inline u32 compute(void) const { return m_Compute; }

		[[nodiscard]] inline operator bool(void) const { return m_Graphics != u32max && m_Compute != u32max; }
	private:
		u32 m_Graphics = u32max, m_Compute = u32max;
	};

	class SurfaceDeviceInfo {
	public:
		SurfaceDeviceInfo(void) = default;
		SurfaceDeviceInfo(vk::raii::PhysicalDevice, const vk::raii::SurfaceKHR&);

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