#ifndef NA2_GRAPHICS_DEVICE_BUFFER_HPP
#define NA2_GRAPHICS_DEVICE_BUFFER_HPP

#include "Natrium2/Graphics/Device.hpp"

namespace Na2::Graphics
{
	enum class BufferTypeFlag : u8 {
		None = 0,

		VertexBuffer  = NA2_BIT(0),
		IndexBuffer   = NA2_BIT(1),
		StorageBuffer = NA2_BIT(2),
		UniformBuffer = NA2_BIT(3),

		TransferSrc   = NA2_BIT(4),
		TransferDst   = NA2_BIT(5),

		All = u8max
	};

	struct BufferCreateInfo {
		View<const Device> device;

		u64 size = 0;
		u64 subbuffer_count = 1;
		bool host_accessible = true;
		BufferTypeFlag type = BufferTypeFlag::None;
	};

	struct BufferCreateInfo2 {
		View<const Device> device;

		vk::DeviceSize size = 0;
		vk::DeviceSize count = 1;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memory_props;
		vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
	};

	class DeviceBuffer {
	public:
		DeviceBuffer(const BufferCreateInfo& info);
		DeviceBuffer(const BufferCreateInfo2& info);

		void destroy(void);
		~DeviceBuffer(void) { this->destroy(); }

		DeviceBuffer(DeviceBuffer&& other) noexcept;
		DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;

		// buffer needs to be host accessible 
		void set_data(const void* data);
		void set_data_ex(const void* data, u64 offset, u64 size);
		void set_subdata(const void* data, u64 index);

		Byte* map(void);
		Byte* map_ex(u64 offset, u64 size);

		void unmap(void);

		[[nodiscard]] inline Byte* mapped(void) const { return m_Mapped; }

		[[nodiscard]] inline u64 element_size(void) const { return m_ElementSize; }
		[[nodiscard]] inline u64 aligned_size(void) const { return m_AlignedSize; }
		[[nodiscard]] inline u64 total_size(void) const { return m_TotalSize; }
		[[nodiscard]] inline u64 subbuffer_count(void) const { return m_SubBufferCount; }

		[[nodiscard]] inline operator bool(void) const { return m_Buffer; }

		[[nodiscard]] inline vk::Buffer buffer(void) const { return m_Buffer; }
		[[nodiscard]] inline vk::DeviceMemory memory(void) const { return m_Memory; }

		[[nodiscard]] BufferTypeFlag type(void) const { return m_Type; }
		[[nodiscard]] bool host_accessible(void) const { return m_HostAccessible; }

	private:
		View<const Device> m_Device = nullptr;

		BufferTypeFlag m_Type = BufferTypeFlag::None;
		bool m_HostAccessible = false;

		vk::Buffer m_Buffer = nullptr;
		vk::DeviceMemory m_Memory = nullptr;

		Byte* m_Mapped = nullptr;

		u64 m_ElementSize = 0;
		u64 m_AlignedSize = 0;
		u64 m_TotalSize = 0;
		u64 m_SubBufferCount = 0;
	};

	inline BufferTypeFlag operator|(BufferTypeFlag lhs, BufferTypeFlag rhs) { return (BufferTypeFlag)((u8)lhs | (u8)rhs); }
	inline BufferTypeFlag operator&(BufferTypeFlag lhs, BufferTypeFlag rhs) { return (BufferTypeFlag)((u8)lhs & (u8)rhs); }
	inline BufferTypeFlag operator^(BufferTypeFlag lhs, BufferTypeFlag rhs) { return (BufferTypeFlag)((u8)lhs ^ (u8)rhs); }
	inline BufferTypeFlag operator~(BufferTypeFlag state) { return (BufferTypeFlag)(~(u8)state); }

	inline BufferTypeFlag& operator|=(BufferTypeFlag& lhs, BufferTypeFlag rhs) { lhs = lhs | rhs; return lhs; }
	inline BufferTypeFlag& operator&=(BufferTypeFlag& lhs, BufferTypeFlag rhs) { lhs = lhs & rhs; return lhs; }
	inline BufferTypeFlag& operator^=(BufferTypeFlag& lhs, BufferTypeFlag rhs) { lhs = lhs ^ rhs; return lhs; }

	inline bool operator==(BufferTypeFlag lhs, BufferTypeFlag rhs) { return (u8)lhs == (u8)rhs; }
	inline bool operator!=(BufferTypeFlag lhs, BufferTypeFlag rhs) { return (u8)lhs != (u8)rhs; }

	inline bool operator!(BufferTypeFlag state) { return (u8)state == 0; }
} // namespace Na2::Graphics

#endif // NA2_GRAPHICS_DEVICE_BUFFER_HPP