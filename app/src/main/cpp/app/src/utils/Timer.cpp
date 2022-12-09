#include "utils/Timer.h"

namespace android_slam
{

Timer::Timer() noexcept
    : m_last_point(std::chrono::steady_clock::now())
{}

float Timer::mark() const noexcept
{
    const TimePoint curr_point = m_last_point;
    m_last_point               = std::chrono::steady_clock::now();
    return Duration(m_last_point - curr_point).count();
}

float Timer::peek() const noexcept
{
    return Duration(std::chrono::steady_clock::now() - m_last_point).count();
}

}  // namespace android_slam