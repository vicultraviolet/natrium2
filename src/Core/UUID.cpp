#include "Pch.hpp"
#include "Natrium2/Core/UUID.hpp"

namespace Na2
{
	static std::random_device randomDevice;
	static std::mt19937 randomGenerator{ randomDevice() };
	static uuids::basic_uuid_random_generator<std::mt19937> uuidGenerator{ randomGenerator };
	static uuids::uuid_name_generator uuidNameGenerator{ uuidGenerator() };

	UUID_t UUID::Generate(void)
	{
		return uuidGenerator();
	}

	UUID_t UUID::Generate(const std::string& name)
	{
		return uuidNameGenerator(name);
	}

	UUID_t UUID::Generate(const std::filesystem::path& path)
	{
		return uuidNameGenerator(path.string());
	}

	UUID_t UUID::FromString(const std::string& uuid_string)
	{
		try
		{
			return UUID_t::from_string(uuid_string).value();
		} catch (...)
		{
			return UUID_t();
		}
	}

	[[nodiscard]] std::string UUID::ToString(const UUID_t& uuid)
	{
		return uuids::to_string(uuid);
	}

	UUID_t UUID::FromBytes(const Byte bytes[])
	{
		return UUID_t(bytes, bytes + 16);
	}

	UUID_t FromBytes(const std::array<Byte, 16>& bytes)
	{
		return UUID_t(bytes);
	}
} // namespace Na2
