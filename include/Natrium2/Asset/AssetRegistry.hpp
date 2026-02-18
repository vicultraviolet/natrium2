#ifndef NA2_ASSET_REGISTRY_HPP
#define NA2_ASSET_REGISTRY_HPP

#include "Natrium2/Asset/SerializedJson.hpp"

namespace Na2
{
	class AssetRegistry : public SerializedJson {
	public:
		template<typename T, typename... t_Args>
		[[nodiscard]] Ref<T> create_asset(const std::string& name, t_Args&&... __args)
		{
			auto it = m_Registry.find(name);
			if (it != m_Registry.end())
			{
				return MakeRef<T>(it->second, std::forward<t_Args>(__args)...);
			}

			UUID_t uuid = UUID::Generate();

			m_Registry.try_emplace(name, uuid);

			return MakeRef<T>(uuid, std::forward<t_Args>(__args)...);
		}

		[[nodiscard]] const UUID_t& get(const std::string& name) const;

		[[nodiscard]] nlohmann::json serialize(void) const override;
		void deserialize(const nlohmann::json& j) override;

		inline void clear(void) { m_Registry.clear(); }

		[[nodiscard]] inline operator bool(void) const override { return true; }
	protected:
		FileErrorCode _handle_missing(const std::filesystem::path& path) override;

	private:
		std::unordered_map<std::string, UUID_t> m_Registry;
	};
} // namespace Na2

#endif // NA2_ASSET_REGISTRY_HPP