#ifndef NA2_ASSET_TEXT_HPP
#define NA2_ASSET_TEXT_HPP

#include "Natrium2/Asset/Asset.hpp"

namespace Na2
{
	class Text : public Asset {
	public:
		Text(void) = default;
		Text(const UUID_t& uuid) : Asset(uuid) {}

		FileErrorCode load(const std::filesystem::path& path) override;
		FileErrorCode save(const std::filesystem::path& path) override;

		[[nodiscard]] inline std::string& str(void) { return m_String; }
		[[nodiscard]] inline const std::string& str(void) const { return m_String; }

		[[nodiscard]] inline operator bool(void) const override { return !m_String.empty(); }
	private:
		std::string m_String;
	};
} // namespace Na2

#endif // NA2_ASSET_TEXT_HPP