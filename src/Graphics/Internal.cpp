#include "Pch.hpp"
#include "./Internal.hpp"

#ifdef NA2_USE_GLFW
	#include <GLFW/glfw3.h>
#endif // NA2_USE_GLFW

namespace Na2::Graphics
{
	vk::raii::SurfaceKHR CreateSurface(const vk::raii::Instance& instance, const Window& window)
	{
		VkSurfaceKHR surface = nullptr;
	#ifdef NA2_USE_GLFW
		VkResult result = glfwCreateWindowSurface(*instance, window.native(), nullptr, &surface);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface!");
	#endif // NA2_USE_GLFW

		return vk::raii::SurfaceKHR(instance, surface);
	}

	[[nodiscard]] static u32 countFlags(vk::QueueFlags flags)
	{
		return (u32)std::bitset<32>((u32)flags).count();
	}

	u32 FindFirstQueueFamily(QueueFamilyFindInfo&& info)
	{
		for (u32 i = 0; const auto& family : info.device.getQueueFamilyProperties())
		{
			if (info.present && !info.device.getSurfaceSupportKHR(i, info.surface))
				continue;

			if (family.queueFlags & info.desired_flags)
				return i;

			i++;
		}

		return u32max;
	}

	u32 FindBestQueueFamily(QueueFamilyFindInfo&& info)
	{
		u32 best_index = u32max,
			min_extra_flags = u32max;

		for (u32 i = 0; const auto& family : info.device.getQueueFamilyProperties())
		{
			vk::QueueFlags flags = family.queueFlags;

			if ((flags & info.desired_flags) == info.desired_flags)
			{
				vk::QueueFlags extra_flags = flags & ~info.desired_flags & info.undesired_flags;
				u32 extra_count = countFlags(extra_flags);

				if (info.present)
				{
					if (!info.device.getSurfaceSupportKHR(i, info.surface))
						continue;
				} else
				{
					if (info.surface && info.device.getSurfaceSupportKHR(i, info.surface))
						extra_count++;
				}

				if (extra_count == 0)
					return i;

				if (extra_count < min_extra_flags)
				{
					min_extra_flags = extra_count;
					best_index = i;
				}

				i++;
			}
		}

		return best_index;
	}

	SurfaceDeviceInfo::SurfaceDeviceInfo(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
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
