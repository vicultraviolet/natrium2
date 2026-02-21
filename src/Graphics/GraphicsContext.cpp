#include "Pch.hpp"
#include "Natrium2/Graphics/GraphicsContext.hpp"

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

	Context::Context(ContextCreateInfo&& info)
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

	static Logger<> logger{ "Vulkan" };
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
			logger.print(Error, data->pMessage);
		#if defined(_MSC_VER)
			__debugbreak();
		#elif defined(__GNUC__) || defined(__clang__)
			__builtin_trap();
		#endif
		} else
		if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			logger.print(Warn, data->pMessage);
		} else
		if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			logger.print(Debug, data->pMessage);
		} else
		{
			logger.print(Trace, data->pMessage);
		}
	#endif // NA2_VK_VALIDATION_LAYERS

		return vk::False;
	}
} // namespace Na2::Graphics
