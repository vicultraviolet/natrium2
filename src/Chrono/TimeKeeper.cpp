#include "Pch.hpp"
#include "Natrium2/Chrono/TimeKeeper.hpp"

namespace Na2::Chrono
{
    TimeKeeper::TimeKeeper(f64 target_fps)
    : m_FrameDuration(1.0 / target_fps),
      m_PreviousTime(std::chrono::steady_clock::now())
    {}

    f64 TimeKeeper::tick(void)
    {
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<f64> delta = current_time - m_PreviousTime;
        m_PreviousTime = current_time;
        return m_DeltaTime = delta.count();
    }

    void TimeKeeper::pace(void) const
    {
        using namespace std::chrono;

        auto next_frame_time = m_PreviousTime + m_FrameDuration;
        auto now = steady_clock::now();

        if (next_frame_time <= now)
            return;

        auto remainder = duration_cast<microseconds>(next_frame_time - now);
        if (remainder < 3ms)
        {
            while (steady_clock::now() < next_frame_time)
                std::this_thread::yield();
        } else
        {
            auto sleep_until = next_frame_time - 2ms;
            std::this_thread::sleep_until(sleep_until);
            while (steady_clock::now() < next_frame_time);
        }
    }

    void TimeKeeper::set_target_fps(f64 target_fps)
    {
        m_FrameDuration = std::chrono::duration<f64>(1.0 / target_fps);
    }
} // namespace Na2::Chrono
