#ifndef NA2_GRAPHICS_RENDERER_HPP
#define NA2_GRAPHICS_RENDERER_HPP

#include "Natrium2/Graphics/Device.hpp"
#include "Natrium2/Graphics/DeviceBuffer.hpp"
#include "Natrium2/Graphics/RenderTarget.hpp"

namespace Na2::Graphics
{
	constexpr u32 k_FramesInFlight = 2;

	struct RendererCreateInfo {
		View<const Device> device;
	};

	struct BufferCopyInfo {
		View<DeviceBuffer> src;
		View<DeviceBuffer> dst;

		u64 src_offset = 0, dst_offset = 0;
		u64 size = dst->total_size();
	};

	class Renderer {
	public:
		Renderer(void) = default;
		Renderer(const RendererCreateInfo& info);

		void destroy(void);
		~Renderer(void) { this->destroy(); }

		Renderer(Renderer&& other) noexcept;
		Renderer& operator=(Renderer&& other) noexcept;

		//void begin_compute(void);
		//void end_compute(void);

		void begin_graphics(RenderTarget& target);
		void end_graphics(RenderTarget& target);

		//void copy_buffer(const BufferCopyInfo& info);

		[[nodiscard]] vk::CommandBuffer begin_transient_cmd_buffer(u32 family);
		void end_transient_cmd_buffer(vk::CommandBuffer, u32 family);

		[[nodiscard]] inline View<const Device> device(void) const { return m_Device; }
		[[nodiscard]] inline operator bool(void) const { return m_Device; }

	private:
		View<const Device> m_Device = nullptr;

		vk::CommandPool m_GraphicsCommandPool = nullptr;
		vk::CommandPool m_ComputeCommandPool = nullptr;

		vk::CommandPool m_TransientGraphicsCommandPool = nullptr;
		vk::CommandPool m_TransientComputeCommandPool = nullptr;

		Array<vk::CommandBuffer, k_FramesInFlight> m_CommandBuffers;
		u32 m_FrameIndex = 0;
	};
}

#endif // NA2_GRAPHICS_RENDERER_HPP