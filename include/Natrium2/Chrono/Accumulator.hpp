#ifndef NA2_CHRONO_ACCUMULATOR_HPP
#define NA2_CHRONO_ACCUMULATOR_HPP

#include "Natrium2/Core.hpp"

namespace Na2::Chrono
{
	class Accumulator {
	public:
		explicit Accumulator(f64 period);

		[[nodiscard]] u64 update(f64 dt);
		inline void reset(void) { m_Accumulator = 0.0; }

		void set_period(f64 period);

		[[nodiscard]] inline f64 accumulator(void) const { return m_Accumulator; }
		[[nodiscard]] inline f64 period(void) const { return m_Period; }
	private:
		f64 m_Accumulator = 0.0, m_Period, m_InversePeriod;
	};
} // namespace Na2::Chrono

#endif // NA2_CHRONO_ACCUMULATOR_HPP