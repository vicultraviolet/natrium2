#include "Pch.hpp"
#include "Natrium2/Chrono/Cooldown.hpp"

namespace Na2::Chrono
{
	Cooldown::Cooldown(f64 recharge_time, u32 max_charges)
	: m_RechargeTime(recharge_time),
	  m_Charges(max_charges),
	  m_MaxCharges(max_charges)
	{}

	void Cooldown::update(f64 dt)
	{
		m_Accumulator += dt;
		while (m_Accumulator >= m_RechargeTime && m_Charges < m_MaxCharges)
		{
			m_Accumulator -= m_RechargeTime;
			m_Charges++;
		}
	}

	void Cooldown::reset(void)
	{
		m_Accumulator = 0.0;
		m_Charges = m_MaxCharges;
	}

	bool Cooldown::try_consume(void)
	{
		if (m_Charges == 0)
			return false;

		m_Charges--;
		return true;
	}

	bool Cooldown::try_consume(u32 amount)
	{
		if (m_Charges < amount)
			return false;

		m_Charges -= amount;
		return true;
	}
} // namespace Na2::Chrono
