#ifndef NA2_GRAPHICS_SWAPCHAIN_HPP
#define NA2_GRAPHICS_SWAPCHAIN_HPP

#include "Natrium2/Graphics/RenderTarget.hpp"
#include "Natrium2/Graphics/DeviceImage.hpp"

namespace Na2::Graphics
{
	class Swapchain : public RenderTarget {
	public:
		Swapchain(void) = default;
		Swapchain(const Device& device, const Window& window);

		void destroy(void);
		~Swapchain(void) { this->destroy(); }

		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;

		[[nodiscard]] inline RenderTargetType type(void) const override { return RenderTargetType::Swapchain; }

		[[nodiscard]] inline vk::SurfaceKHR surface(void) const { return m_Surface; }
		[[nodiscard]] inline vk::SurfaceFormatKHR surface_format(void) const { return m_SurfaceFormat; }

		[[nodiscard]] inline vk::SwapchainKHR swapchain(void) const { return m_Swapchain; }

		[[nodiscard]] inline const auto& imgs(void) const { return m_Images; }
		[[nodiscard]] inline const auto& img_views(void) const { return m_ImageViews; }

	private:
		View<const Device> m_Device = nullptr;

		vk::SurfaceKHR m_Surface = nullptr;
		vk::SurfaceFormatKHR m_SurfaceFormat{};

		vk::SwapchainKHR m_Swapchain = nullptr;

		ArrayList<vk::Image> m_Images;
		ArrayList<vk::ImageView> m_ImageViews;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_SWAPCHAIN_HPP