#include "Pch.hpp"
#include "Natrium2/Graphics/Swapchain.hpp"

#include "./Internal.hpp"

namespace Na2::Graphics
{
	static vk::SurfaceFormatKHR pickSurfaceFormat(const ArrayList<vk::SurfaceFormatKHR>& formats);
	static vk::PresentModeKHR pickPresentMode(const ArrayList<vk::PresentModeKHR>& present_modes);
	static vk::Extent2D pickResolution(const vk::SurfaceCapabilitiesKHR& capabilities, u32 window_width, u32 window_height);

	Swapchain::Swapchain(const Device& device, const Window& window)
	: m_Device(&device)
 	{
		m_Surface = CreateSurface(m_Device->context()->instance(), window);

		m_Width = window.width();
		m_Height = window.height();

		m_Viewport = vk::Viewport
		{
			.x        = 0.0f,
			.y        = (float)m_Height,
			.width    = (float)m_Width,
			.height   = -(float)m_Height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		m_Scissor = vk::Rect2D
		{
			.offset = {
				.x = 0,
				.y = 0
			},
			.extent = {
				.width  = m_Width,
				.height = m_Height
			}
		};

		SurfaceDeviceInfo surface_info(m_Device->physical_device(), m_Surface);

		m_Width = surface_info.capabilities().currentExtent.width;
		m_Height = surface_info.capabilities().currentExtent.height;
		m_SurfaceFormat = pickSurfaceFormat(surface_info.formats());

		vk::SwapchainCreateInfoKHR swapchain_info
		{
			.surface = m_Surface,

			.minImageCount =
				(surface_info.capabilities().maxImageCount > 0 &&
				 surface_info.capabilities().minImageCount >=
					surface_info.capabilities().maxImageCount) ?
				surface_info.capabilities().maxImageCount :
				surface_info.capabilities().minImageCount + 1,

			.imageFormat = m_SurfaceFormat.format,
			.imageColorSpace = m_SurfaceFormat.colorSpace,
			.imageExtent = surface_info.capabilities().currentExtent,

			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,

			.imageSharingMode =
				m_Device->graphics_queue() == m_Device->present_queue() ?
				vk::SharingMode::eExclusive :
				vk::SharingMode::eConcurrent,

			.preTransform = surface_info.capabilities().currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = pickPresentMode(surface_info.present_modes()),
			.clipped = true,
		};

		m_Swapchain = m_Device->logical_device().createSwapchainKHR(swapchain_info);

		m_Images = m_Device->logical_device().getSwapchainImagesKHR(m_Swapchain);

		m_ImageViews.reallocate(m_Images.size());
		for (vk::Image img : m_Images)
			m_ImageViews.emplace(CreateImageView(
				device,
				img,
				vk::ImageAspectFlagBits::eColor,
				m_SurfaceFormat.format,
				1 // layer count
			));
	}

	static vk::SurfaceFormatKHR pickSurfaceFormat(const ArrayList<vk::SurfaceFormatKHR>& formats)
	{
		for (vk::SurfaceFormatKHR format : formats)
			if (format.format == vk::Format::eR8G8B8A8Uint &&
				format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return format;

		return formats[0];
	}

	static vk::PresentModeKHR pickPresentMode(const ArrayList<vk::PresentModeKHR>& present_modes)
	{
		for (vk::PresentModeKHR present_mode : present_modes)
			if (present_mode == vk::PresentModeKHR::eMailbox)
				return present_mode;

		return vk::PresentModeKHR::eFifo;
	}

	static vk::Extent2D pickResolution(const vk::SurfaceCapabilitiesKHR& capabilities, u32 window_width, u32 window_height)
	{
		if (capabilities.currentExtent.width != u32max)
			return capabilities.currentExtent;

		return vk::Extent2D{
			.width = std::clamp(
				window_width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width
			),
			.height = std::clamp(
				window_height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height
			),
		};
	}

	void Swapchain::destroy(void)
	{
		for (vk::ImageView img_view : m_ImageViews)
			m_Device->logical_device().destroyImageView(img_view);

		m_Images.clear();

		if (m_Swapchain)
		{
			m_Device->logical_device().destroySwapchainKHR(m_Swapchain);
			m_Swapchain = nullptr;
		}

		m_SurfaceFormat = {};

		if (m_Surface)
		{
			m_Device->context()->instance().destroySurfaceKHR(m_Surface);
			m_Surface = nullptr;
		}
	}

	Swapchain::Swapchain(Swapchain&& other) noexcept
	: m_Surface(std::move(other.m_Surface)),
	  m_SurfaceFormat(std::exchange(other.m_SurfaceFormat, {})),

	  m_Swapchain(std::move(other.m_Swapchain)),

	  m_Images(std::move(other.m_Images)),
	  m_ImageViews(std::move(other.m_ImageViews))
	{}

	Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->destroy();

		m_Surface = std::move(other.m_Surface);
		m_SurfaceFormat = std::exchange(other.m_SurfaceFormat, {});

		m_Swapchain = std::move(other.m_Swapchain);

		m_Images = std::move(other.m_Images);
		m_ImageViews = std::move(other.m_ImageViews);

		return *this;
	}
} // namespace Na2::Graphics
