#include "Pch.hpp"
#include "Natrium2/Chrono/Accumulator.hpp"

namespace Na2::Chrono
{
	Accumulator::Accumulator(f64 period)
	: m_Period(period),
	  m_InversePeriod(1.0 / period)
	{}

	u64 Accumulator::update(f64 dt)
	{
		m_Accumulator += dt;
		u64 count = (u64)(m_Accumulator * m_InversePeriod);
		m_Accumulator -= (f64)count * m_Period;  
		return count;
	}

	void Accumulator::set_period(f64 period)
	{
		m_Period = period;
		m_InversePeriod = 1.0 / period;
	}
} // namespace Na2::Chrono
