#include "Pch.hpp"
#include "Natrium2/Graphics/DeviceImage.hpp"

namespace Na2::Graphics
{
	vk::ImageView CreateImageView(
		const Device& device,
		vk::Image img,
		vk::ImageAspectFlags aspect_mask,
		vk::Format format,
		u32 layer_count
	)
	{
		vk::ImageViewCreateInfo create_info
		{
			.image = img,
			.viewType = layer_count > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D,
			.format = format,

			.subresourceRange = {
				.aspectMask = aspect_mask,

				.baseMipLevel = 0,
				.levelCount = 1,

				.baseArrayLayer = 0,
				.layerCount = layer_count,
			}
		};

		return device.logical_device().createImageView(create_info);
	}

} // namespace Na2::Graphics
