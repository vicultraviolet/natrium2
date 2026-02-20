#ifndef NA2_CHRONO_TIME_KEEPER_HPP
#define NA2_CHRONO_TIME_KEEPER_HPP

#include "Natrium2/Core.hpp"

namespace Na2::Chrono
{
    class TimeKeeper {
    public:
        explicit TimeKeeper(f64 target_fps = 60.0);

        f64 tick(void);
        void pace(void) const;

        void set_target_fps(f64 target_fps);

        [[nodiscard]] inline f64 dt(void) const { return m_DeltaTime; }
        [[nodiscard]] inline f64 delta_time(void) const { return m_DeltaTime; }
    private:
        std::chrono::duration<f64> m_FrameDuration;   
        std::chrono::steady_clock::time_point m_PreviousTime;
        f64 m_DeltaTime = 0.0;
    };
} // namespace Na2::Chrono

#endif // NA2_CHRONO_TIME_KEEPER_HPP