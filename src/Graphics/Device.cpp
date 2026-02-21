#include "Pch.hpp"
#include "Natrium2/Graphics/Device.hpp"

#include "./Internal.hpp"

#include "Natrium2/Graphics/GraphicsContext.hpp"

namespace Na2::Graphics
{
	static i32 ratePhysicalDevice(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	);

	static bool queueFamiliesSupported(vk::raii::PhysicalDevice device);

	static bool requiredExtensionsSupported(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	);

	Device::Device(DeviceCreateInfo&& info)
	: m_Extensions(std::move(info.extensions))
	{
		const auto& instance = info.context->m_Instance;

		ArrayList<const char*> extensions_vk(4);

		if (m_Extensions.contains(DeviceExtension::Swapchain))
		{
			extensions_vk.emplace(vk::KHRSwapchainExtensionName);
			extensions_vk.emplace(vk::KHRMaintenance1ExtensionName);
		}

		i32 high_score = 0;
		for (const auto& device : instance.enumeratePhysicalDevices())
		{
			i32 score = ratePhysicalDevice(device, extensions_vk);
			if (score > high_score)
			{
				high_score = score;
				m_PhysicalDevice = device;
			}
		}

		NA2_VERIFY(*m_PhysicalDevice, "No suitable GPU!");

		vk::raii::SurfaceKHR temp_surface = CreateSurface(instance, info.window);

		m_GraphicsQueueFamily = FindBestQueueFamily({
			.device = m_PhysicalDevice,
			.desired_flags = vk::QueueFlagBits::eGraphics,
			.undesired_flags = vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer
		});
		m_PresentQueueFamily = FindBestQueueFamily({
			.device = m_PhysicalDevice,
			.present = true,
			.surface = temp_surface
		});
		m_ComputeQueueFamily  = FindBestQueueFamily({
			.device = m_PhysicalDevice,
			.desired_flags = vk::QueueFlagBits::eCompute,
			.undesired_flags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer
		});
		m_TransferQueueFamily = FindBestQueueFamily({
			.device = m_PhysicalDevice,
			.desired_flags = vk::QueueFlagBits::eTransfer,
			.undesired_flags = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute
		});

		g_Logger.printf(Info, "Selected graphics queue family index: {}", m_GraphicsQueueFamily);
		g_Logger.printf(Info, "Selected present queue family index: {}",  m_PresentQueueFamily);
		g_Logger.printf(Info, "Selected compute queue family index: {}",  m_ComputeQueueFamily);
		g_Logger.printf(Info, "Selected transfer queue family index: {}", m_TransferQueueFamily);

		float queue_priorities[] = { 1.0f };

		std::set<u32> unique_queue_families = {
			m_GraphicsQueueFamily, m_PresentQueueFamily, m_ComputeQueueFamily, m_TransferQueueFamily
		};

		ArrayList<vk::DeviceQueueCreateInfo> queue_infos(4);

		for (u32 unique_queue_family : unique_queue_families)
			queue_infos.emplace(vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = unique_queue_family,
				.queueCount = 1,
				.pQueuePriorities = queue_priorities
			});

		vk::PhysicalDeviceFeatures device_features
		{
			.sampleRateShading = vk::True,
			.samplerAnisotropy = vk::True
		};

		vk::DeviceCreateInfo device_info
		{
			//.pNext =,

			.queueCreateInfoCount = (u32)queue_infos.size(),
			.pQueueCreateInfos = queue_infos.ptr(),

			.enabledExtensionCount = (u32)extensions_vk.size(),
			.ppEnabledExtensionNames = extensions_vk.ptr(),

			.pEnabledFeatures = &device_features
		};

		m_LogicalDevice = vk::raii::Device(m_PhysicalDevice, device_info);
	}

	static i32 ratePhysicalDevice(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	)
	{
		if (!*device)
			return -1;

		auto properties = device.getProperties();
		auto features = device.getFeatures();

		if (!features.geometryShader)
			return 0;

		if (!queueFamiliesSupported(device))
			return 0;

		if (!features.samplerAnisotropy)
			return 0;

		if (!requiredExtensionsSupported(device, extensions))
			return 0;

		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			properties.limits.maxImageDimension2D += 1000;

		return properties.limits.maxImageDimension2D;
	}

	static bool queueFamiliesSupported(vk::raii::PhysicalDevice device)
	{
		bool graphics = false, compute = false, transfer = false;

		auto families = device.getQueueFamilyProperties();
		for (const auto& family : families)
		{
			if (family.queueFlags & vk::QueueFlagBits::eGraphics)
				graphics = true;

			if (family.queueFlags & vk::QueueFlagBits::eCompute)
				compute = true;

			if (family.queueFlags & vk::QueueFlagBits::eTransfer)
				transfer = true;

			if (graphics && compute && transfer)
				return true;
		}

		return graphics && compute && transfer;
	}

	static bool requiredExtensionsSupported(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	)
	{
		auto available_extensions = device.enumerateDeviceExtensionProperties();
		std::set<std::string_view> required_extensions(extensions.begin(), extensions.end());

		for (const auto& extension : available_extensions)
			required_extensions.erase(extension.extensionName);

		return required_extensions.empty();
	}
} // namespace Na2::Graphics