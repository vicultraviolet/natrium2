#include "Pch.hpp"
#include "Natrium2/Graphics/Renderer.hpp"

#include "Natrium2/Graphics/Swapchain.hpp"

namespace Na2::Graphics
{
	Renderer::Renderer(const RendererCreateInfo& info)
	: m_Device(info.device)
	{
		m_GraphicsCommandPool = m_Device->logical_device().createCommandPool(vk::CommandPoolCreateInfo{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = m_Device->graphics_queue_family()
		});

		m_TransientGraphicsCommandPool = m_Device->logical_device().createCommandPool(vk::CommandPoolCreateInfo{
			.flags = vk::CommandPoolCreateFlagBits::eTransient,
			.queueFamilyIndex = m_Device->graphics_queue_family()
		});

		if (m_Device->graphics_queue_family() == m_Device->compute_queue_family())
		{
			m_ComputeCommandPool = m_GraphicsCommandPool;
			m_TransientComputeCommandPool = m_TransientGraphicsCommandPool;
		} else
		{
			m_ComputeCommandPool = m_Device->logical_device().createCommandPool(vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = m_Device->compute_queue_family()
			});

			m_TransientComputeCommandPool = m_Device->logical_device().createCommandPool(vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eTransient,
				.queueFamilyIndex = m_Device->compute_queue_family()
			});
		}

		vk::CommandBufferAllocateInfo cmd_buffer_info
		{
			.commandPool = m_GraphicsCommandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = k_FramesInFlight
		};
		(void)m_Device->logical_device().allocateCommandBuffers(&cmd_buffer_info, m_CommandBuffers.ptr());

		m_CommandBuffers.set_size(k_FramesInFlight);
	}

	void Renderer::destroy(void)
	{
		m_FrameIndex = 0;

		m_CommandBuffers.clear();

		if (m_TransientComputeCommandPool)
		{
			m_Device->logical_device().destroyCommandPool(m_TransientComputeCommandPool);
			m_TransientComputeCommandPool = nullptr;
		}

		if (m_TransientGraphicsCommandPool)
		{
			m_Device->logical_device().destroyCommandPool(m_TransientGraphicsCommandPool);
			m_TransientGraphicsCommandPool = nullptr;
		}

		if (m_ComputeCommandPool)
		{
			m_Device->logical_device().destroyCommandPool(m_ComputeCommandPool);
			m_ComputeCommandPool = nullptr;
		}

		if (m_GraphicsCommandPool)
		{
			m_Device->logical_device().destroyCommandPool(m_GraphicsCommandPool);
			m_GraphicsCommandPool = nullptr;
		}

		m_Device = nullptr;
	}

	void Renderer::begin_graphics(RenderTarget& target)
	{
		vk::CommandBuffer cmd_buffer = m_CommandBuffers[m_FrameIndex];

		cmd_buffer.reset();

		vk::CommandBufferBeginInfo begin_info{};
		cmd_buffer.begin(begin_info);

		switch (target.type())
		{
			case RenderTargetType::Swapchain:
			{
				auto& swapchain = (Swapchain&)target;

				struct {
					vk::ClearValue color;

					operator vk::ClearValue*(void) { return (vk::ClearValue*)this; }
				} clear_values;

				clear_values.color.color = std::array<float, 4>{ 0.3f, 0.1f, 0.4f, 1.0f };

				vk::RenderPassBeginInfo render_pass_info
				{
					.renderPass = swapchain.render_pass(),
					.framebuffer = swapchain.current_framebuffer(),

					.renderArea = vk::Rect2D
					{
						.offset = vk::Offset2D
						{
							.x = 0,
							.y = 0
						},
						.extent = vk::Extent2D
						{
							.width = swapchain.width(),
							.height = swapchain.height()
						}
					},

					.clearValueCount = 1,
					.pClearValues = clear_values
				};

				cmd_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

				cmd_buffer.setViewport(0, { swapchain.viewport() });
				cmd_buffer.setScissor(0, { swapchain.scissor() });

				break;
			}
		default:
			throw std::runtime_error("Failed to begin render pass: Unknown render target type!");
		}
	}

	void Renderer::end_graphics(RenderTarget& target)
	{
		vk::CommandBuffer cmd_buffer = m_CommandBuffers[m_FrameIndex];

		cmd_buffer.endRenderPass();

		cmd_buffer.end();

		vk::SubmitInfo submit_info
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer
		};
		vk::Fence fence = nullptr;

		switch (target.type())
		{
			case RenderTargetType::Swapchain:
			{
				auto& swapchain = (Swapchain&)target;
				const auto& fd = swapchain.current_frame_data();

				vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

				submit_info.waitSemaphoreCount = 1;
				submit_info.pWaitSemaphores = &fd.image_available_semaphore;

				submit_info.pWaitDstStageMask = &wait_stage;

				submit_info.signalSemaphoreCount = 1;
				submit_info.pSignalSemaphores = &fd.render_finished_semaphore;

				fence = fd.in_flight_fence;

				break;
			}
		default:
			throw std::runtime_error("Failed to begin render pass: Unknown render target type!");
		}

		m_Device->graphics_queue().submit({ submit_info }, fence);

		m_FrameIndex = (m_FrameIndex + 1) % k_FramesInFlight;
	}

	/*
	void Renderer::begin_compute(void)
	{

	}

	void Renderer::end_compute(void)
	{

	}
	*/

	/*
	void Renderer::copy_buffer(const BufferCopyInfo& info)
	{
		auto cmd_buffer = this->begin_transient_cmd_buffer(m_Device->transfer_queue_family());

		vk::BufferCopy copy_region
		{
			.srcOffset = info.src_offset,
			.dstOffset = info.dst_offset,
			.size = info.size
		};
		cmd_buffer.copyBuffer(info.src->buffer(), info.dst->buffer(), { copy_region });

		this->end_transient_cmd_buffer(cmd_buffer, m_Device->transfer_queue_family());
	}
	*/

	[[nodiscard]] vk::CommandBuffer Renderer::begin_transient_cmd_buffer(u32 family)
	{
		vk::CommandPool cmd_pool = nullptr;
		if (family == m_Device->graphics_queue_family())
		{
			cmd_pool = m_TransientGraphicsCommandPool;
		} else
		if (family == m_Device->compute_queue_family())
		{
			cmd_pool = m_TransientComputeCommandPool;
		} else
		{
			throw std::runtime_error("Failed to begin transient command buffer: Invalid queue family!");
		}

		vk::CommandBufferAllocateInfo alloc_info
		{
			.commandPool = cmd_pool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		vk::CommandBuffer cmd_buffer;
		(void)m_Device->logical_device().allocateCommandBuffers(&alloc_info, &cmd_buffer);

		vk::CommandBufferBeginInfo begin_info
		{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};

		cmd_buffer.begin(begin_info);

		return cmd_buffer;
	}

	void Renderer::end_transient_cmd_buffer(vk::CommandBuffer cmd_buffer, u32 family)
	{
		vk::CommandPool cmd_pool = nullptr;
		if (family == m_Device->graphics_queue_family())
		{
			cmd_pool = m_TransientGraphicsCommandPool;
		} else
		if (family == m_Device->compute_queue_family())
		{
			cmd_pool = m_TransientComputeCommandPool;
		} else
		{
			throw std::runtime_error("Failed to begin transient command buffer: Invalid queue family!");
		}

		cmd_buffer.end();

		vk::SubmitInfo submit_info
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer
		};

		m_Device->queue(family).submit({ submit_info });
		m_Device->queue(family).waitIdle();

		m_Device->logical_device().freeCommandBuffers(cmd_pool, { cmd_buffer });
	}

	Renderer::Renderer(Renderer&& other) noexcept
	: m_Device(std::move(other.m_Device)),

	  m_GraphicsCommandPool(std::move(other.m_GraphicsCommandPool)),
	  m_ComputeCommandPool(std::move(other.m_ComputeCommandPool)),

	  m_TransientGraphicsCommandPool(std::move(other.m_TransientGraphicsCommandPool)),
	  m_TransientComputeCommandPool(std::move(other.m_TransientComputeCommandPool)),

	  m_CommandBuffers(std::move(other.m_CommandBuffers)),

	  m_FrameIndex(std::exchange(other.m_FrameIndex, 0))
	{

	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->destroy();

		m_Device = std::move(other.m_Device);

		m_GraphicsCommandPool = std::move(other.m_GraphicsCommandPool);
		m_ComputeCommandPool = std::move(other.m_ComputeCommandPool);

		m_TransientGraphicsCommandPool = std::move(other.m_TransientGraphicsCommandPool);
		m_TransientComputeCommandPool = std::move(other.m_TransientComputeCommandPool);

		m_CommandBuffers = std::move(other.m_CommandBuffers);

		m_FrameIndex = std::exchange(other.m_FrameIndex, 0);

		return *this;
	}
} // namespace Na2::Graphics
