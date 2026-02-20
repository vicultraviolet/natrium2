#include "Pch.hpp"
#include "Natrium2/Graphics/GraphicsContext.hpp"

#include "./Internal.hpp"

#ifdef NA2_USE_GLFW
	#include <GLFW/glfw3.h>
#endif // NA2_USE_GLFW

namespace Na2::Graphics
{
	static bool validationLayersSupported(const ArrayList<const char*>& requested_layers);

	static vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo(void);
	static vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* data,
		void* user_data
	);

	static i32 ratePhysicalDevice(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	);

	static bool requiredExtensionsSupported(
		vk::raii::PhysicalDevice device,
		const ArrayList<const char*>& extensions
	);

	static bool queueFamiliesSupported(vk::raii::PhysicalDevice device);

	Context::Context(ContextCreateInfo&& info)
	: m_DeviceExtensions(std::move(info.extensions))
	{
#ifdef NA2_VK_VALIDATION_LAYERS
		const ArrayList<const char*>& validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		if (!validationLayersSupported(validation_layers))
			throw std::runtime_error("Validation layers requested, but not supported!");
#endif // NA2_VK_VALIDATION_LAYERS

		vk::ApplicationInfo app_info
		{
			.pApplicationName = "",
			.applicationVersion = vk::makeApiVersion(0, 1, 0, 0),
			.pEngineName = "Natrium2",
			.engineVersion = vk::makeApiVersion(0, 1, 0, 0),
			.apiVersion = vk::ApiVersion12
		};

#ifdef NA2_USE_GLFW
		u32 glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		ArrayList<const char*> instance_extensions((u64)glfw_extension_count + 1);

		for (u32 i = 0; i < glfw_extension_count; i++)
			instance_extensions.emplace(glfw_extensions[i]);
#else 
		ArrayList<const char*> instance_extensions(1);
#endif // NA2_USE_GLFW

		vk::InstanceCreateInfo instance_info;

#ifdef NA2_VK_VALIDATION_LAYERS
		auto debug_messenger_info = debugMessengerInfo();
		instance_info.pNext = &debug_messenger_info;

		instance_info.enabledLayerCount = (u32)validation_layers.size();
		instance_info.ppEnabledLayerNames = validation_layers.ptr();
		
		instance_extensions.emplace(vk::EXTDebugUtilsExtensionName);
#endif // NA2_VK_VALIDATION_LAYERS

		instance_info.pApplicationInfo = &app_info;

		instance_info.enabledExtensionCount = (u32)instance_extensions.size();
		instance_info.ppEnabledExtensionNames = instance_extensions.ptr();

		m_Instance = vk::raii::Instance(m_Context, instance_info);

#ifdef NA2_VK_VALIDATION_LAYERS
		m_DebugMessenger = vk::raii::DebugUtilsMessengerEXT(m_Instance, debug_messenger_info);
#endif // NA2_VK_VALIDATION_LAYERS

		if (m_DeviceExtensions.contains(DeviceExtension::Swapchain))
		{
			m_VkDeviceExtensions.emplace(vk::KHRSwapchainExtensionName);
			m_VkDeviceExtensions.emplace(vk::KHRMaintenance1ExtensionName);
		}

		i32 high_score = -1;
		for (const auto& device : m_Instance.enumeratePhysicalDevices())
		{
			i32 score = ratePhysicalDevice(device, m_VkDeviceExtensions);
			if (score > high_score)
			{
				high_score = score;
				m_PhysicalDevice = device;
			}
		}
	}

	static bool validationLayersSupported(const ArrayList<const char*>& requested_layers)
	{
		auto available_layers = vk::enumerateInstanceLayerProperties();
		if (available_layers.empty())
			return false;

		for (const char* requested_layer : requested_layers)
		{
			bool found = false;

			for (const vk::LayerProperties& available_layer : available_layers)
			{
				if (!strcmp(requested_layer, available_layer.layerName))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				g_Logger.printf(Error, "Validation layer requested, but not supported: {}", requested_layer);
				return false;
			}
		}

		return true;
	}

	static vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo(void)
	{
		return vk::DebugUtilsMessengerCreateInfoEXT
		{
			.messageSeverity =
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
			.messageType =
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = debugCallback
		};
	}

	static vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* data,
		void* user_data
	)
	{
	#ifdef NA2_VK_VALIDATION_LAYERS
		if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			g_Logger.print(Error, data->pMessage);
		#if defined(_MSC_VER)
			__debugbreak();
		#elif defined(__GNUC__) || defined(__clang__)
			__builtin_trap();
		#endif
		} else
		if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			g_Logger.print(Warn, data->pMessage);
		} else
		if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			g_Logger.print(Debug, data->pMessage);
		} else
		{
			g_Logger.print(Trace, data->pMessage);
		}
	#endif // NA2_VK_VALIDATION_LAYERS

		return vk::False;
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

	static bool queueFamiliesSupported(vk::raii::PhysicalDevice device)
	{
		bool graphics = false, compute = false;

		auto families = device.getQueueFamilyProperties();
		for (const auto& family : families)
		{
			if (family.queueFlags & vk::QueueFlagBits::eGraphics)
				graphics = true;

			if (family.queueFlags & vk::QueueFlagBits::eCompute)
				compute = true;

			if (graphics && compute)
				return true;
		}

		return graphics && compute;
	}
} // namespace Na2::Graphics
