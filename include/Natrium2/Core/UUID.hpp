#ifndef NA2_UUID_HPP
#define NA2_UUID_HPP

#include "Natrium2/Core.hpp"

namespace Na2
{
	using UUID_t = uuids::uuid;

	namespace UUID
	{
		inline constexpr UUID_t k_Null;

		[[nodiscard]] UUID_t Generate(void);
		[[nodiscard]] UUID_t Generate(const std::string& name);
		[[nodiscard]] UUID_t Generate(const std::filesystem::path& path);

		[[nodiscard]] UUID_t FromString(const std::string& uuid_string);
		[[nodiscard]] std::string ToString(const UUID_t& uuid);

		[[nodiscard]] UUID_t FromBytes(const Byte bytes[]);
		[[nodiscard]] UUID_t FromBytes(const std::array<Byte, 16>& bytes);

	} // namespace UUID
} // namespace Na2

#endif // NA2_UUID_HPP