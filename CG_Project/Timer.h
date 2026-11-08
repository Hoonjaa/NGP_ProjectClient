#pragma once

#include <chrono>

class Timer
{
private:
    using Clock = std::chrono::steady_clock;

    double Cycle; // 실행 주기
    double cumulativeTime; // 누적 시간

    Clock::time_point previousTime; // 이전 시간

    void (*callback)();
public:
    Timer(double cycle, void (*func)());

    void Update();
    void Reset();
    void SetCycle(double cycle);
};

