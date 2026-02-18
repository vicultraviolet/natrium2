#ifndef NA2_ASSET_SERIALIZED_JSON_HPP
#define NA2_ASSET_SERIALIZED_JSON_HPP

#include "Natrium2/Asset/Asset.hpp"

namespace Na2
{
	class SerializedJson : public Asset {
	public:
		SerializedJson(void) = default;
		SerializedJson(const UUID_t& uuid) : Asset(uuid) {}

		FileErrorCode load(const std::filesystem::path& path) override;
		FileErrorCode save(const std::filesystem::path& path) override;

		[[nodiscard]] virtual nlohmann::json serialize(void) const = 0;
		virtual void deserialize(const nlohmann::json& j) = 0;

		[[nodiscard]] virtual inline std::string to_string(void) const { return this->serialize().dump(4); }
	};
} // namespace Na2

#endif // NA2_ASSET_SERIALIZED_JSON_HPP