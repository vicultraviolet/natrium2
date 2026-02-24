#ifndef NA2_GRAPHICS_RENDER_TARGET_HPP
#define NA2_GRAPHICS_RENDER_TARGET_HPP

#include "Natrium2/Graphics/Device.hpp"

namespace Na2::Graphics
{
	enum class RenderTargetType : u8 {
		None = 0,
		Swapchain
	};

	class RenderTarget {
	public:
		RenderTarget(void) = default;
		virtual ~RenderTarget(void) = default;

		[[nodiscard]] inline u32 width(void)  const { return m_Width; }
		[[nodiscard]] inline u32 height(void) const { return m_Height; }

		[[nodiscard]] inline const auto& viewport(void) const { return m_Viewport; }
		[[nodiscard]] inline const auto& scissor(void)  const { return m_Scissor; }

		[[nodiscard]] virtual RenderTargetType type(void) const = 0;
	protected:
		u32 m_Width = 0, m_Height = 0;
		vk::Viewport m_Viewport;
		vk::Rect2D m_Scissor;
	};
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_RENDER_TARGET_HPP