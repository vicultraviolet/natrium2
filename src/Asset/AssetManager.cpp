#include "Pch.hpp"
#include "Natrium2/Asset/AssetManager.hpp"

namespace Na2
{
	AssetManager::AssetManager(const AssetManagerCreateInfo& info)
	: m_AssetRegistryPath(info.asset_registry_path)
	{
		m_Registry.load(info.asset_registry_path);
	}

	void AssetManager::destroy(void)
	{
		m_Assets.clear();
		m_Registry.clear();
	}

	void AssetManager::destroy_asset(const UUID_t& uuid)
	{
		m_Assets.erase(uuid);
	}

	Ref<Asset> AssetManager::get(const UUID_t& uuid) const
	{
		if (uuid.is_nil())
			return nullptr;

		auto it = m_Assets.find(uuid);
		if (it != m_Assets.end())
			return it->second;

		return nullptr;
	}

	Ref<Asset> AssetManager::get_by_name(const std::string& name) const
	{
		return this->get(this->get_uuid_by_name(name));
	}
} // namespace Na2
