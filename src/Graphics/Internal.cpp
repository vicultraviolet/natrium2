#include "Pch.hpp"
#include "./Internal.hpp"

#ifdef NA2_USE_GLFW
	#include <GLFW/glfw3.h>
#endif // NA2_USE_GLFW

namespace Na2::Graphics
{
	vk::raii::SurfaceKHR CreateSurface(const vk::raii::Instance& instance, Window& window)
	{
		VkSurfaceKHR surface = nullptr;
	#ifdef NA2_USE_GLFW
		VkResult result = glfwCreateWindowSurface(*instance, window.native(), nullptr, &surface);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
	#endif // NA2_USE_GLFW

		return vk::raii::SurfaceKHR(instance, surface);
	}

	QueueFamilyIndices::QueueFamilyIndices(
		vk::raii::PhysicalDevice device,
		const vk::raii::SurfaceKHR& surface
	)
	{
		auto properties = device.getQueueFamilyProperties();

		for (u32 i = 0; const auto& property : properties)
		{
			if (property.queueFlags & vk::QueueFlagBits::eGraphics)
				if (device.getSurfaceSupportKHR(i, surface))
					m_Graphics = i;

			if (property.queueFlags & vk::QueueFlagBits::eCompute)
				m_Compute = i;

			if (*this)
				break;

			i++;
		}
	}

	SurfaceSupport::SurfaceSupport(
		vk::raii::PhysicalDevice _device,
		const vk::raii::SurfaceKHR& surface
	)
	{
		vk::PhysicalDevice device = _device;

		m_Capabilities = device.getSurfaceCapabilitiesKHR(surface);

		u32 format_count;
		(void)device.getSurfaceFormatsKHR(surface, &format_count, nullptr);
		m_Formats.reallocate(format_count, format_count);
		(void)device.getSurfaceFormatsKHR(surface, &format_count, m_Formats.ptr());

		u32 present_mode_count;
		(void)device.getSurfacePresentModesKHR(surface, &present_mode_count, nullptr);
		m_PresentModes.reallocate(present_mode_count, present_mode_count);
		(void)device.getSurfacePresentModesKHR(surface, &present_mode_count, m_PresentModes.ptr());
	}
} // Na2::Graphics
