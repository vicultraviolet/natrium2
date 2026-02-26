#include "Pch.hpp"
#include "Natrium2/Graphics/DeviceBuffer.hpp"

namespace Na2::Graphics
{
	static vk::BufferUsageFlags bufferTypeToVk(BufferTypeFlag type)
	{
		vk::BufferUsageFlags usage;

		if ((type & BufferTypeFlag::StorageBuffer) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eStorageBuffer;

		if ((type & BufferTypeFlag::UniformBuffer) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eUniformBuffer;

		if ((type & BufferTypeFlag::IndexBuffer) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eIndexBuffer;

		if ((type & BufferTypeFlag::VertexBuffer) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eVertexBuffer;

		if ((type & BufferTypeFlag::TransferDst) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eTransferDst;

		if ((type & BufferTypeFlag::TransferSrc) != BufferTypeFlag::None)
			usage |= vk::BufferUsageFlagBits::eTransferSrc;

		return usage;
	}

	static u32 findMemoryType(
		vk::PhysicalDevice physical_device,
		u32 type_filter,
		vk::MemoryPropertyFlags user_props
	)
	{
		vk::PhysicalDeviceMemoryProperties memory_props = physical_device.getMemoryProperties();

		for (u32 i = 0; i < memory_props.memoryTypeCount; i++)
			if ((type_filter & (1 << i)) && (memory_props.memoryTypes[i].propertyFlags & user_props) == user_props)
				return i;

		return 0;
	}

	DeviceBuffer::DeviceBuffer(const BufferCreateInfo& info)
	: m_Device(info.device),
	  m_Type(info.type),
	  m_HostAccessible(info.host_accessible),
	  m_ElementSize(info.size),
	  m_SubBufferCount(info.subbuffer_count)
	{
		if ((info.type & BufferTypeFlag::UniformBuffer | BufferTypeFlag::StorageBuffer) != BufferTypeFlag::None)
		{
			auto physical_device_props = m_Device->physical_device().getProperties();

			vk::DeviceSize alignment = std::max(
				physical_device_props.limits.minStorageBufferOffsetAlignment,
				physical_device_props.limits.minUniformBufferOffsetAlignment
			);

			m_AlignedSize = (info.size + alignment - 1) & ~(alignment - 1);
		} else
		{
			m_AlignedSize = info.size;
		}

		m_TotalSize = m_AlignedSize * info.subbuffer_count;

		vk::BufferUsageFlags usage = bufferTypeToVk(info.type);
		vk::MemoryPropertyFlags memory_props;

		if (info.host_accessible)
		{
			memory_props |= vk::MemoryPropertyFlagBits::eHostVisible;
			memory_props |= vk::MemoryPropertyFlagBits::eHostCoherent;
		} else
		{
			memory_props |= vk::MemoryPropertyFlagBits::eDeviceLocal;

			if (info.type != BufferTypeFlag::None)
				usage |= vk::BufferUsageFlagBits::eTransferDst;
		}

		vk::BufferCreateInfo buffer_info
		{
			.size = m_TotalSize,
			.usage = usage,
			.sharingMode = vk::SharingMode::eExclusive
		};

		m_Buffer = m_Device->logical_device().createBuffer(buffer_info);

		auto memory_requirements = m_Device->logical_device().getBufferMemoryRequirements(m_Buffer);

		vk::MemoryAllocateInfo memory_info
		{
			.allocationSize = memory_requirements.size,
			.memoryTypeIndex = findMemoryType(
				m_Device->physical_device(),
				memory_requirements.memoryTypeBits,
				memory_props
			)
		};

		m_Memory = m_Device->logical_device().allocateMemory(memory_info);
		m_Device->logical_device().bindBufferMemory(m_Buffer, m_Memory, 0);
	}

	DeviceBuffer::DeviceBuffer(const BufferCreateInfo2& info)
	: m_Device(info.device),
	  m_TotalSize(info.size * info.count),
	  m_SubBufferCount(info.count),
	  m_ElementSize(info.size),
	  m_AlignedSize(info.size)
	{
		vk::BufferCreateInfo buffer_info
		{
			.size = m_TotalSize,
			.usage = info.usage,
			.sharingMode = info.sharing_mode
		};
		m_Buffer = m_Device->logical_device().createBuffer(buffer_info);
		auto memory_requirements = m_Device->logical_device().getBufferMemoryRequirements(m_Buffer);

		vk::MemoryAllocateInfo memory_info
		{
			.allocationSize = memory_requirements.size,
			.memoryTypeIndex = findMemoryType(
				m_Device->physical_device(),
				memory_requirements.memoryTypeBits,
				info.memory_props
			)
		};
		m_Memory = m_Device->logical_device().allocateMemory(memory_info);
		m_Device->logical_device().bindBufferMemory(m_Buffer, m_Memory, 0);
	}

	void DeviceBuffer::destroy(void)
	{
		m_SubBufferCount = 0;
		m_TotalSize = 0;
		m_AlignedSize = 0;
		m_ElementSize = 0;

		this->unmap();

		if (m_Memory)
		{
			m_Device->logical_device().freeMemory(m_Memory);
			m_Memory = nullptr;
		}

		if (m_Buffer)
		{
			m_Device->logical_device().destroyBuffer(m_Buffer);
			m_Buffer = nullptr;
		}

		m_HostAccessible = false;
		m_Type = BufferTypeFlag::None;

		m_Device = nullptr;
	}

	void DeviceBuffer::set_data(const void* data)
	{
		this->set_data_ex(data, 0, m_TotalSize);
	}

	void DeviceBuffer::set_data_ex(const void* data, u64 offset, u64 size)
	{
		if (!data)
			return;

		NA2_ASSERT(
			m_HostAccessible,
			"Failed to set the DeviceBuffer's data: Memory is not host accessible!\n"
			"\tTo set the data on a device local buffer, see "
		);

		if (m_Mapped)
		{
			memcpy(m_Mapped + offset, data, size);
		} else
		{
			this->map_ex(offset, size);
			memcpy(m_Mapped, data, size);
			this->unmap();
		}
	}

	void DeviceBuffer::set_subdata(const void* data, u64 index)
	{
		this->set_data_ex(data, index * m_AlignedSize, m_ElementSize);
	}

	Byte* DeviceBuffer::map(void)
	{
		return this->map_ex(0, m_TotalSize);
	}

	Byte* DeviceBuffer::map_ex(u64 offset, u64 size)
	{
		return m_Mapped = (Byte*)m_Device->logical_device().mapMemory(m_Memory, offset, size);
	}

	void DeviceBuffer::unmap(void)
	{
		m_Device->logical_device().unmapMemory(m_Memory);
		m_Mapped = nullptr;
	}

	DeviceBuffer::DeviceBuffer(DeviceBuffer&& other) noexcept
	: m_Device(std::exchange(other.m_Device, nullptr)),

	  m_Type(std::exchange(other.m_Type, BufferTypeFlag::None)),
	  m_HostAccessible(std::exchange(other.m_HostAccessible, false)),

	  m_Buffer(std::move(other.m_Buffer)),
	  m_Memory(std::move(other.m_Memory)),

	  m_Mapped(std::exchange(other.m_Mapped, nullptr)),

	  m_ElementSize(std::exchange(m_ElementSize, 0)),
	  m_AlignedSize(std::exchange(m_AlignedSize, 0)),
	  m_TotalSize(std::exchange(m_TotalSize, 0)),
	  m_SubBufferCount(std::exchange(m_SubBufferCount, 0))
	{}

	DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->unmap();

		if (m_Memory)
		{
			m_Device->logical_device().freeMemory(m_Memory);
			m_Memory = nullptr;
		}

		if (m_Buffer)
		{
			m_Device->logical_device().destroyBuffer(m_Buffer);
			m_Buffer = nullptr;
		}

		m_Device = std::exchange(other.m_Device, nullptr);

		m_Type = std::exchange(other.m_Type, BufferTypeFlag::None);
		m_HostAccessible = std::exchange(other.m_HostAccessible, false);

		m_Buffer = std::move(other.m_Buffer);
		m_Memory = std::move(other.m_Memory);

		m_Mapped = std::exchange(other.m_Mapped, nullptr);

		m_ElementSize = std::exchange(m_ElementSize, 0);
		m_AlignedSize = std::exchange(m_AlignedSize, 0);
		m_TotalSize = std::exchange(m_TotalSize, 0);
		m_SubBufferCount = std::exchange(m_SubBufferCount, 0);

		return *this;
	}
} // namespace Na2::Graphics
