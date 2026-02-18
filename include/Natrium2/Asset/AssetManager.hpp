#ifndef NA2_ASSET_MANAGER_HPP
#define NA2_ASSET_MANAGER_HPP

#include "Natrium2/Asset/Asset.hpp"
#include "Natrium2/Asset/AssetRegistry.hpp"

namespace Na2
{
	struct AssetManagerCreateInfo {
		std::filesystem::path asset_registry_path = "assets/asset_registry.json";
	};

	class AssetManager {
	public:
		AssetManager(void) = default;
		AssetManager(const AssetManagerCreateInfo& info);

		~AssetManager(void) { this->destroy(); }
		void destroy(void);

		template<typename T, typename... t_Args>
		[[nodiscard]] Ref<T> create_asset(const std::string& name, t_Args&&... __args)
		{
			if (auto asset = this->get_by_name(name))
			{
				return dynamic_ref_cast<T>(asset);
			}

			Ref<T> asset = m_Registry.create_asset<T>(name, std::forward<t_Args>(__args)...);

			m_Assets.try_emplace(asset->uuid(), asset);

			return asset;
		}

		void destroy_asset(const UUID_t& uuid);

		[[nodiscard]] Ref<Asset> get(const UUID_t& uuid) const;
		[[nodiscard]] Ref<Asset> get_by_name(const std::string& name) const;

		template<typename T>
		[[nodiscard]] Ref<T> get(const UUID_t& uuid) const
		{
			return dynamic_ref_cast<T>(this->get(uuid));
		}

		template<typename T>
		[[nodiscard]] Ref<T> get_by_name(const std::string& name) const
		{
			return dynamic_ref_cast<T>(this->get_by_name(name));
		}

		[[nodiscard]] inline const UUID_t& get_uuid_by_name(const std::string& name) const { return m_Registry.get(name); }

		inline void save_registry(void) { m_Registry.save(m_AssetRegistryPath); }

		[[nodiscard]] inline const auto& asset_registry_path(void) const { return m_AssetRegistryPath; }
	private:
		AssetRegistry m_Registry;
		std::unordered_map<UUID_t, Ref<Asset>> m_Assets;

		std::filesystem::path m_AssetRegistryPath;
	};
} // namespace Na2

#endif // NA2_ASSET_MANAGER_HPP