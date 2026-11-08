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
    tpsElapsed += deltaTime;

    if (cumulativeTime >= Cycle)
    {
        cumulativeTime -= Cycle;

        if (callback) {
            callback();
            tickCount++;
        }
    }

    if (tpsElapsed >= 1.0)
    {
        tps = static_cast<int>(tickCount / tpsElapsed);

        tickCount = 0;
        tpsElapsed = 0.0;
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

int Timer::GetTPS() const
{
    return tps;
}