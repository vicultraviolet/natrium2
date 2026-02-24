#ifndef NA2_GRAPHICS_SWAPCHAIN_HPP
#define NA2_GRAPHICS_SWAPCHAIN_HPP

#include "Natrium2/Graphics/RenderTarget.hpp"
#include "Natrium2/Graphics/Renderer.hpp"
#include "Natrium2/Graphics/DeviceImage.hpp"

namespace Na2::Graphics
{
	class Swapchain : public RenderTarget {
	public:
		struct FrameData {
			vk::Semaphore image_available_semaphore = nullptr;
			vk::Semaphore render_finished_semaphore = nullptr;
			vk::Fence     in_flight_fence = nullptr;
		};

		Swapchain(void) = default;
		Swapchain(const Device& device, const Window& window);

		void destroy(void);
		~Swapchain(void) { this->destroy(); }

		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;
		
		void recreate_swapchain(void);

		[[nodiscard]] bool acquire_next_image(void);
		void present(void);

		[[nodiscard]] inline RenderTargetType type(void) const override { return RenderTargetType::Swapchain; }

		[[nodiscard]] inline vk::SurfaceKHR surface(void) const { return m_Surface; }
		[[nodiscard]] inline vk::SurfaceFormatKHR surface_format(void) const { return m_SurfaceFormat; }

		[[nodiscard]] inline vk::SwapchainKHR swapchain(void) const { return m_Swapchain; }

		[[nodiscard]] inline const auto& imgs(void) const { return m_Images; }
		[[nodiscard]] inline const auto& img_views(void) const { return m_ImageViews; }

		[[nodiscard]] inline vk::RenderPass render_pass(void) const { return m_RenderPass; }

		[[nodiscard]] inline const auto& framebuffers(void) const { return m_Framebuffers; }
		[[nodiscard]] inline vk::Framebuffer current_framebuffer(void) const { return m_Framebuffers[m_ImageIndex]; }

		[[nodiscard]] inline const auto& frame_datas(void) const { return m_FrameDatas; }
		[[nodiscard]] inline const FrameData& current_frame_data(void) const { return m_FrameDatas[m_FrameIndex]; }

		[[nodiscard]] inline const auto& image_in_flight_fences(void) const { return m_ImageInFlightFences; }
		[[nodiscard]] inline vk::Fence current_image_in_flight_fence(void) const { return m_ImageInFlightFences[m_ImageIndex]; }

		[[nodiscard]] inline u32 img_index(void) const { return m_ImageIndex; }
		[[nodiscard]] inline u32 frame_index(void) const { return m_FrameIndex; }

	private:
		void _create_surface(void);
		void _update_viewport(void);
		void _create_swapchain(void);
		void _create_img_views(void);
		void _create_render_pass(void);
		void _create_framebuffers(void);
		void _create_sync_objects(void);
	private:
		View<const Device> m_Device = nullptr;
		View<const Window> m_Window = nullptr;

		vk::SurfaceKHR m_Surface = nullptr;
		vk::SurfaceFormatKHR m_SurfaceFormat{};

		vk::SwapchainKHR m_Swapchain = nullptr;

		ArrayList<vk::Image> m_Images;
		ArrayList<vk::ImageView> m_ImageViews;

		vk::RenderPass m_RenderPass = nullptr;

		ArrayList<vk::Framebuffer> m_Framebuffers;

		Array<FrameData, k_FramesInFlight> m_FrameDatas;
		ArrayList<vk::Fence> m_ImageInFlightFences;

		u32 m_ImageIndex = 0, m_FrameIndex = 0;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_SWAPCHAIN_HPP