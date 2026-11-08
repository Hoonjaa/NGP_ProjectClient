#include "Timer.h"

Timer::Timer(double hz, void (*func)())
    : Cycle(1.0 / 60.0),
    cumulativeTime(0.0),
    previousTime(Clock::now()),
    callback(func)
{
    SetCycle(hz);
}

void Timer::Update()
{
    auto currentTime = Clock::now();

    double deltaTime = std::chrono::duration<double>( currentTime - previousTime).count();

    previousTime = currentTime;
    cumulativeTime += deltaTime;

    if (cumulativeTime >= Cycle)
    {
        cumulativeTime -= Cycle;

        if (callback)
            callback();
    }
}

void Timer::Reset()
{
    cumulativeTime = 0.0;
    previousTime = Clock::now();
}

void Timer::SetCycle(double cycle)
{
    if (cycle <= 0.0)
        return;

    Cycle = 1.0 / cycle;
}