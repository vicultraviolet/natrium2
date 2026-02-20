#ifndef NA2_CHRONO_COOLDOWN_HPP
#define NA2_CHRONO_COOLDOWN_HPP

#include "Natrium2/Core.hpp"

namespace Na2::Chrono
{
	class Cooldown {
	public:
		explicit Cooldown(
			f64 recharge_time,
			u32 max_charges = 1
		);

		void update(f64 dt);
		void reset(void);

		[[nodiscard]] bool try_consume(void);
		[[nodiscard]] bool try_consume(u32 amount);

		inline void set_recharge_time(f64 recharge_time) { m_RechargeTime = recharge_time; }
		inline void set_max_charges(u32 max_charges) { m_MaxCharges = max_charges; }

		[[nodiscard]] inline f64 accumulator(void) const { return m_Accumulator; }
		[[nodiscard]] inline f64 recharge_time(void) const { return m_RechargeTime; }
		[[nodiscard]] inline u32 charges(void) const { return m_Charges; }
		[[nodiscard]] inline u32 max_charges(void) const { return m_MaxCharges; }
	private:
		f64 m_Accumulator = 0.0, m_RechargeTime;
		u32 m_Charges, m_MaxCharges;
	};
} // namespace Na2::Chrono

#endif // NA2_CHRONO_COOLDOWN_HPP