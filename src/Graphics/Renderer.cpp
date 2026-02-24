#include "Pch.hpp"
#include "Natrium2/Graphics/Renderer.hpp"

#include "Natrium2/Graphics/Swapchain.hpp"

namespace Na2::Graphics
{
	Renderer::Renderer(const RendererCreateInfo& info)
	: m_Device(info.device),
	  m_DoGraphics(info.do_graphics),
	  m_DoCompute(info.do_compute)
	{
		vk::Device logical_device = m_Device->logical_device();

		std::vector<vk::CommandBuffer> graphics_cmd_buffers;

		if (info.do_graphics)
		{
			m_GraphicsCommandPool = logical_device.createCommandPool(vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = m_Device->graphics_queue_family()
			});

			m_TransientGraphicsCommandPool = logical_device.createCommandPool(vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eTransient,
				.queueFamilyIndex = m_Device->graphics_queue_family()
			});

			vk::CommandBufferAllocateInfo cmd_buffer_info
			{
				.commandPool = m_GraphicsCommandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = k_FramesInFlight
			};

			graphics_cmd_buffers = logical_device.allocateCommandBuffers(cmd_buffer_info);
		}

		std::vector<vk::CommandBuffer> compute_cmd_buffers;

		if (info.do_compute)
		{
			if (info.do_graphics &&
				m_Device->graphics_queue_family() == m_Device->compute_queue_family())
			{
				m_ComputeCommandPool = m_GraphicsCommandPool;
				m_TransientComputeCommandPool = m_TransientGraphicsCommandPool;
			} else
			{
				m_ComputeCommandPool = logical_device.createCommandPool(vk::CommandPoolCreateInfo
				{
					.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
					.queueFamilyIndex = m_Device->compute_queue_family()
				});

				m_TransientComputeCommandPool = logical_device.createCommandPool(vk::CommandPoolCreateInfo
				{
					.flags = vk::CommandPoolCreateFlagBits::eTransient,
					.queueFamilyIndex = m_Device->compute_queue_family()
				});
			}

			vk::CommandBufferAllocateInfo cmd_buffer_info2
			{
				.commandPool = m_ComputeCommandPool,
				.level = vk::CommandBufferLevel::ePrimary,
				.commandBufferCount = k_FramesInFlight
			};

			compute_cmd_buffers = logical_device.allocateCommandBuffers(cmd_buffer_info2);
		}

		vk::SemaphoreCreateInfo semaphore_info{};

		vk::FenceCreateInfo fence_info
		{
			.flags = vk::FenceCreateFlagBits::eSignaled
		};

		for (u32 i = 0; i < k_FramesInFlight; i++)
		{
			FrameData fd;

			if (info.do_graphics)
				fd.graphics_cmd_buffer = graphics_cmd_buffers[i];

			if (info.do_compute)
			{
				fd.compute_cmd_buffer = compute_cmd_buffers[i];
				fd.compute_finished_semaphore = logical_device.createSemaphore(semaphore_info);
				fd.compute_fence = logical_device.createFence(fence_info);
			}

			m_FrameDatas.emplace(std::move(fd));
		}
	}

	void Renderer::destroy(void)
	{
		m_FrameIndex = 0;

		for (const auto& fd : m_FrameDatas)
		{
			if (fd.compute_fence)
				m_Device->logical_device().destroyFence(fd.compute_fence);

			if (fd.compute_finished_semaphore)
				m_Device->logical_device().destroySemaphore(fd.compute_finished_semaphore);
		}

		m_FrameDatas.clear();

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

		m_DoCompute = false;
		m_DoGraphics = false;

		m_Device = nullptr;
	}

	void Renderer::begin_compute(void)
	{
		const auto& fd = m_FrameDatas[m_FrameIndex];
		vk::CommandBuffer cmd_buffer = fd.compute_cmd_buffer;

		(void)m_Device->logical_device().waitForFences(
			{ fd.compute_fence },
			vk::True, // wait all
			u64max // timeout
		);
		m_Device->logical_device().resetFences({ fd.compute_fence });

		cmd_buffer.reset();

		vk::CommandBufferBeginInfo begin_info{};
		cmd_buffer.begin(begin_info);
	}

	void Renderer::end_compute(void)
	{
		static u32 x_LastFrame = u32max;

		const auto& fd = m_FrameDatas[m_FrameIndex];

		vk::CommandBuffer cmd_buffer = fd.compute_cmd_buffer;
		cmd_buffer.end();

		vk::SubmitInfo submit_info
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd_buffer,
		};

		if (m_DoGraphics)
		{
			if (x_LastFrame != m_FrameIndex)
			{
				submit_info.signalSemaphoreCount = 1;
				submit_info.pSignalSemaphores = &fd.compute_finished_semaphore;
			}
		}

		m_Device->compute_queue().submit({ submit_info }, fd.compute_fence);

		x_LastFrame = m_FrameIndex;
	}

	void Renderer::begin_graphics(RenderTarget& target)
	{
		m_DoGraphics = true;
		
		const auto& fd = m_FrameDatas[m_FrameIndex];
		vk::CommandBuffer cmd_buffer = fd.graphics_cmd_buffer;

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
		const auto& fd = m_FrameDatas[m_FrameIndex];
		vk::CommandBuffer cmd_buffer = fd.graphics_cmd_buffer;

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
				const auto& swapchain_fd = swapchain.current_frame_data();

				Array<vk::Semaphore, 2> wait_semaphores =
				{
					swapchain_fd.image_available_semaphore
				};

				Array<vk::PipelineStageFlags, 2> wait_stages =
				{
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
				};

				if (m_DoCompute)
				{
					wait_semaphores.emplace(fd.compute_finished_semaphore);
					wait_stages.emplace(vk::PipelineStageFlagBits::eVertexShader);
				}

				submit_info.waitSemaphoreCount = (u32)wait_semaphores.size();
				submit_info.pWaitSemaphores = wait_semaphores.ptr();
				submit_info.pWaitDstStageMask = wait_stages.ptr();

				submit_info.signalSemaphoreCount = 1;
				submit_info.pSignalSemaphores = &swapchain_fd.render_finished_semaphore;

				fence = swapchain_fd.in_flight_fence;

				break;
			}
		default:
			throw std::runtime_error("Failed to begin render pass: Unknown render target type!");
		}

		m_Device->graphics_queue().submit({ submit_info }, fence);
	}

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

	  m_DoGraphics(std::exchange(other.m_DoGraphics, false)),
	  m_DoCompute(std::exchange(other.m_DoCompute, false)),

	  m_GraphicsCommandPool(std::move(other.m_GraphicsCommandPool)),
	  m_ComputeCommandPool(std::move(other.m_ComputeCommandPool)),

	  m_TransientGraphicsCommandPool(std::move(other.m_TransientGraphicsCommandPool)),
	  m_TransientComputeCommandPool(std::move(other.m_TransientComputeCommandPool)),

	  m_FrameDatas(std::move(other.m_FrameDatas)),

	  m_FrameIndex(std::exchange(other.m_FrameIndex, 0))
	{

	}

	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->destroy();

		m_Device = std::move(other.m_Device);

		m_DoGraphics = std::exchange(other.m_DoGraphics, false);
		m_DoCompute = std::exchange(other.m_DoCompute, false);

		m_GraphicsCommandPool = std::move(other.m_GraphicsCommandPool);
		m_ComputeCommandPool = std::move(other.m_ComputeCommandPool);

		m_TransientGraphicsCommandPool = std::move(other.m_TransientGraphicsCommandPool);
		m_TransientComputeCommandPool = std::move(other.m_TransientComputeCommandPool);

		m_FrameDatas = std::move(other.m_FrameDatas);

		m_FrameIndex = std::exchange(other.m_FrameIndex, 0);

		return *this;
	}
} // namespace Na2::Graphics
