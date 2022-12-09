#pragma once
#include <chrono>

namespace android_slam
{

// 基于chrono的时间API
class Timer
{
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration  = std::chrono::duration<float>;

public:
    Timer() noexcept;
    float mark() const noexcept;
    float peek() const noexcept;

private:
    mutable TimePoint m_last_point;
};

}  // namespace android_slam