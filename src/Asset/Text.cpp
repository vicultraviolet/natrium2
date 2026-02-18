#include "Pch.hpp"
#include "Natrium2/Asset/Text.hpp"

namespace Na2
{
	FileErrorCode Text::load(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			return FileErrorCode::NotFound;
		}

		if (!std::filesystem::is_regular_file(path))
		{
			return FileErrorCode::InvalidFormat;
		}

		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file)
		{
			return FileErrorCode::Unknown;
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		m_String = ss.str();

		return FileErrorCode::None;
	}

	FileErrorCode Text::save(const std::filesystem::path& path)
	{
		if (std::filesystem::exists(path) &&
			!std::filesystem::is_regular_file(path))
		{
			return FileErrorCode::InvalidFormat;
		}

		std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);
		if (!file)
		{
			return FileErrorCode::Unknown;
		}

		file << m_String;

		return FileErrorCode::None;
	}
} // namespace Na2
