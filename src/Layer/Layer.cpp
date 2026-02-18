#include "Pch.hpp"
#include "Natrium2/Layer/Layer.hpp"

namespace Na2
{
	bool Layer::enabled(void) const
	{
		return (m_State & LayerStateFlag::Enabled) != LayerStateFlag::None;
	}

	bool Layer::updatable(void) const
	{
		return (m_State & LayerStateFlag::Updatable) != LayerStateFlag::None;
	}

	bool Layer::visible(void) const
	{
		return (m_State & LayerStateFlag::Visible) != LayerStateFlag::None;
	}

	void Layer::set_enabled(bool enabled)
	{
		if (enabled)
			m_State |= LayerStateFlag::Enabled;
		else
			m_State &= ~LayerStateFlag::Enabled;
	}

	void Layer::set_updatable(bool updatable)
	{
		if (updatable)
			m_State |= LayerStateFlag::Updatable;
		else
			m_State &= ~LayerStateFlag::Updatable;
	}

	void Layer::set_visible(bool visible)
	{
		if (visible)
			m_State |= LayerStateFlag::Visible;
		else
			m_State &= ~LayerStateFlag::Visible;
	}
} // namespace Na2
