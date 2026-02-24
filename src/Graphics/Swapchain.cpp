#include "Pch.hpp"
#include "Natrium2/Graphics/Swapchain.hpp"

#include "./Internal.hpp"

namespace Na2::Graphics
{
	static vk::SurfaceFormatKHR pickSurfaceFormat(const ArrayList<vk::SurfaceFormatKHR>& formats);
	static vk::PresentModeKHR pickPresentMode(const ArrayList<vk::PresentModeKHR>& present_modes);
	static vk::Extent2D pickResolution(const vk::SurfaceCapabilitiesKHR& capabilities, u32 window_width, u32 window_height);

	Swapchain::Swapchain(const Device& device, const Window& window)
	: m_Device(&device),
	  m_Window(&window)
 	{
		this->_create_surface();
		this->_update_viewport();
		this->_create_swapchain();
		this->_create_img_views();
		this->_create_render_pass();
		this->_create_framebuffers();
		this->_create_sync_objects();
	}

	void Swapchain::_create_surface(void)
	{
		m_Surface = CreateSurface(m_Device->context()->instance(), *m_Window);

		m_Width = m_Window->width();
		m_Height = m_Window->height();
	}

	void Swapchain::_update_viewport(void)
	{
		m_Viewport = vk::Viewport
		{
			.x = 0.0f,
			.y = (float)m_Height,
			.width = (float)m_Width,
			.height = -(float)m_Height,
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
				.width = m_Width,
				.height = m_Height
			}
		};
	}

	void Swapchain::_create_swapchain(void)
	{
		SurfaceDeviceInfo surface_info(m_Device->physical_device(), m_Surface);

		m_Width = surface_info.capabilities().currentExtent.width;
		m_Height = surface_info.capabilities().currentExtent.height;
		m_SurfaceFormat = pickSurfaceFormat(surface_info.formats());

		Array<u32, 2> queue_indices = {
			m_Device->graphics_queue_family(),
			m_Device->present_queue_family()
		};

		bool multi_queue = m_Device->graphics_queue() == m_Device->present_queue();

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

			.imageSharingMode = multi_queue ?
				vk::SharingMode::eExclusive :
				vk::SharingMode::eConcurrent,

			.queueFamilyIndexCount = multi_queue ? 2u : 0u,
			.pQueueFamilyIndices = queue_indices.ptr(),

			.preTransform = surface_info.capabilities().currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = pickPresentMode(surface_info.present_modes()),
			.clipped = true,
		};

		m_Swapchain = m_Device->logical_device().createSwapchainKHR(swapchain_info);

		u32 img_count = 0;
		(void)m_Device->logical_device().getSwapchainImagesKHR(m_Swapchain, &img_count, nullptr);
		m_Images.reallocate(img_count, img_count);
		(void)m_Device->logical_device().getSwapchainImagesKHR(m_Swapchain, &img_count, m_Images.ptr());

		m_ImageInFlightFences.reallocate(img_count);
		for (u32 i = 0; i < img_count; i++)
			m_ImageInFlightFences.emplace(nullptr);
	}

	void Swapchain::_create_img_views(void)
	{
		m_ImageViews.reallocate(m_Images.size());
		for (vk::Image img : m_Images)
			m_ImageViews.emplace(CreateImageView(
				m_Device,
				img,
				vk::ImageAspectFlagBits::eColor,
				m_SurfaceFormat.format,
				1 // layer count
			));
	}

	void Swapchain::_create_render_pass(void)
	{
		vk::AttachmentDescription color_attachment
		{
			.format = m_SurfaceFormat.format,
			.samples = vk::SampleCountFlagBits::e1,

			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,

			.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,

			.initialLayout = vk::ImageLayout::eUndefined,
			.finalLayout = vk::ImageLayout::ePresentSrcKHR
		};

		vk::AttachmentReference color_attachment_ref
		{
			.attachment = 0,
			.layout = vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::SubpassDescription subpass
		{
			.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,

			.colorAttachmentCount = 1,
			.pColorAttachments = &color_attachment_ref,
		};

		vk::RenderPassCreateInfo render_pass_info
		{
			.attachmentCount = 1,
			.pAttachments = &color_attachment,

			.subpassCount = 1,
			.pSubpasses = &subpass
		};

		m_RenderPass = m_Device->logical_device().createRenderPass(render_pass_info);
	}

	void Swapchain::_create_framebuffers(void)
	{
		m_Framebuffers.reallocate(m_ImageViews.size());
		for (u64 i = 0; i < m_ImageViews.size(); i++)
		{
			vk::FramebufferCreateInfo framebuffer_info
			{
				.renderPass = m_RenderPass,

				.attachmentCount = 1,
				.pAttachments = &m_ImageViews[i],

				.width = m_Width,
				.height = m_Height,
				.layers = 1
			};

			m_Framebuffers.emplace(
				m_Device->logical_device().createFramebuffer(framebuffer_info)
			);
		}
	}

	void Swapchain::_create_sync_objects(void)
	{
		vk::SemaphoreCreateInfo semaphore_info{};

		vk::FenceCreateInfo fence_info
		{
			.flags = vk::FenceCreateFlagBits::eSignaled
		};

		for (u32 i = 0; i < k_FramesInFlight; i++)
		{
			m_FrameDatas.emplace(FrameData{
				.image_available_semaphore =
					m_Device->logical_device().createSemaphore(semaphore_info),

				.render_finished_semaphore =
					m_Device->logical_device().createSemaphore(semaphore_info),

				.in_flight_fence = m_Device->logical_device().createFence(fence_info)
			});
		}
	}

	void Swapchain::destroy(void)
	{
		m_FrameIndex = 0;
		m_ImageIndex = 0;

		m_ImageInFlightFences.destroy();

		for (const FrameData& fd : m_FrameDatas)
		{
			m_Device->logical_device().destroyFence(fd.in_flight_fence);
			m_Device->logical_device().destroySemaphore(fd.render_finished_semaphore);
			m_Device->logical_device().destroySemaphore(fd.image_available_semaphore);
		}

		m_FrameDatas.clear();

		for (vk::Framebuffer framebuffer : m_Framebuffers)
			m_Device->logical_device().destroyFramebuffer(framebuffer);

		m_Framebuffers.destroy();

		if (m_RenderPass)
		{
			m_Device->logical_device().destroyRenderPass(m_RenderPass);
			m_RenderPass = nullptr;
		}

		for (vk::ImageView img_view : m_ImageViews)
			m_Device->logical_device().destroyImageView(img_view);

		m_ImageViews.destroy();

		m_Images.destroy();

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

	void Swapchain::recreate_swapchain(void)
	{
		m_Width = m_Window->width();
		m_Height = m_Window->height();

		this->_update_viewport();

		m_Device->logical_device().waitIdle();

		for (auto& framebuffer : m_Framebuffers)
			m_Device->logical_device().destroyFramebuffer(framebuffer);

		m_Framebuffers.clear();

		for (auto& img_view : m_ImageViews)
			m_Device->logical_device().destroyImageView(img_view);

		m_ImageViews.clear();

		m_Images.clear();

		m_Device->logical_device().destroySwapchainKHR(m_Swapchain);

		this->_create_swapchain();
		this->_create_img_views();
		this->_create_framebuffers();
	}

	bool Swapchain::acquire_next_image(void)
	{
		const FrameData& fd = m_FrameDatas[m_FrameIndex];

		if (m_Window->width()  != m_Width ||
			m_Window->height() != m_Height)
		{
			this->recreate_swapchain();
			return false;
		}

		vk::Result result{};

		result = m_Device->logical_device().waitForFences(
			{ fd.in_flight_fence },
			vk::True, // wait all
			u64max // timeout
		);
		NA2_VERIFY_VK(
			result,
			"Failed to begin frame #{} with image #{}:"
			"Error in waiting for fence!",
				m_FrameIndex,
				m_ImageIndex
		);

		result = m_Device->logical_device().acquireNextImageKHR(
			m_Swapchain,
			u64max, // timeout
			fd.image_available_semaphore,
			nullptr, // fence
			&m_ImageIndex
		);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			this->recreate_swapchain();
			return false;
		} else
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
			throw std::runtime_error("Failed to acquire next swapchain image!");

		if (m_ImageInFlightFences[m_ImageIndex])
		{
			result = m_Device->logical_device().waitForFences(
				{ m_ImageInFlightFences[m_ImageIndex] },
				vk::True, // wait all
				u64max // timeout
			);
			NA2_VERIFY_VK(result, "Failed to acquire next swapchain image: Error in waiting for fence!");
		}
		m_ImageInFlightFences[m_ImageIndex] = fd.in_flight_fence;

		m_Device->logical_device().resetFences({ fd.in_flight_fence });

		return true;
	}

	void Swapchain::present(void)
	{
		const FrameData& fd = m_FrameDatas[m_FrameIndex];

		vk::PresentInfoKHR present_info
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &fd.render_finished_semaphore,

			.swapchainCount = 1,
			.pSwapchains = &m_Swapchain,
			.pImageIndices = &m_ImageIndex,
		};

		try
		{
			vk::Result result = m_Device->present_queue().presentKHR(present_info);
			switch (result)
			{
			case vk::Result::eSuboptimalKHR:
				this->recreate_swapchain();
				break;
			case vk::Result::eErrorOutOfDateKHR:
				this->recreate_swapchain();
				break;
			case vk::Result::eSuccess:
				break;
			default:
				NA2_VERIFY_VK(result, "Error in presenting to graphics queue!");
			}

		} catch (const vk::OutOfDateKHRError& err)
		{
			(void)err;
			this->recreate_swapchain();
		}

		m_FrameIndex = (m_FrameIndex + 1) % k_FramesInFlight;
	}

	Swapchain::Swapchain(Swapchain&& other) noexcept
	: m_Surface(std::move(other.m_Surface)),
	  m_SurfaceFormat(std::exchange(other.m_SurfaceFormat, {})),

	  m_Swapchain(std::move(other.m_Swapchain)),

	  m_Images(std::move(other.m_Images)),
	  m_ImageViews(std::move(other.m_ImageViews)),

	  m_RenderPass(std::move(other.m_RenderPass)),

	  m_Framebuffers(std::move(other.m_Framebuffers)),

	  m_FrameDatas(std::move(other.m_FrameDatas)),
	  m_ImageInFlightFences(std::move(other.m_ImageInFlightFences)),

	  m_ImageIndex(std::exchange(other.m_ImageIndex, 0)),
	  m_FrameIndex(std::exchange(other.m_FrameIndex, 0))
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

		m_RenderPass = std::move(other.m_RenderPass);

		m_Framebuffers = std::move(other.m_Framebuffers);

		m_FrameDatas = std::move(other.m_FrameDatas);
		m_ImageInFlightFences = std::move(other.m_ImageInFlightFences);

		m_ImageIndex = std::exchange(other.m_ImageIndex, 0);
		m_FrameIndex = std::exchange(other.m_FrameIndex, 0);

		return *this;
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
} // namespace Na2::Graphics
