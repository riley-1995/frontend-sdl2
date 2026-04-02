#include "FPSLimiter.h"

#include <SDL2/SDL.h>

namespace
{
constexpr uint32_t kMillisecondsPerSecond = 1000;  // Convert FPS to millisecond frame duration
constexpr uint32_t kFrameHistorySize = 10;         // Rolling average window for FPS calculation
}  // namespace

void FPSLimiter::TargetFPS(int fps)
{
    if (fps)
    {
        _targetFrameTime = kMillisecondsPerSecond / fps;
    }
    else
    {
        _targetFrameTime = 0;
    }
}

float FPSLimiter::FPS() const
{
    uint32_t frameTimeSum{ 0 };
    uint32_t frameTimeCount{ 0 };

    for (auto _lastFrameTime : _lastFrameTimes)
    {
        if (_lastFrameTime > 0)
        {
            frameTimeCount++;
            frameTimeSum += _lastFrameTime;
        }
    }

    if (frameTimeCount == 0)
    {
        return 0.0f;
    }

   return static_cast<float>(kMillisecondsPerSecond) / (static_cast<float>(frameTimeSum) / static_cast<float>(frameTimeCount));
}

void FPSLimiter::StartFrame()
{
    _lastTickCount = SDL_GetTicks();
}

void FPSLimiter::EndFrame()
{
    uint32_t frameTime = SDL_GetTicks() - _lastTickCount;

    if (_targetFrameTime && frameTime < _targetFrameTime)
    {
        SDL_Delay(_targetFrameTime - frameTime);
        frameTime = SDL_GetTicks() - _lastTickCount;
    }

    _lastFrameTimes[_nextFrameTimesOffset] = frameTime;
    _nextFrameTimesOffset = (_nextFrameTimesOffset + 1) % kFrameHistorySize;
}
