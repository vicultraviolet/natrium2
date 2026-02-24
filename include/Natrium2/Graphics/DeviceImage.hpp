#ifndef NA2_GRAPHICS_DEVICE_IMAGE_HPP
#define NA2_GRAPHICS_DEVICE_IMAGE_HPP

#include "Natrium2/Graphics/Device.hpp"

namespace Na2::Graphics
{
	vk::ImageView CreateImageView(
		View<const Device> device,
		vk::Image img,
		vk::ImageAspectFlags aspect_mask,
		vk::Format format,
		u32 layer_count
	);
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_DEVICE_IMAGE_HPP