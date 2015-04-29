#include "Timer.h"

using namespace Core;

Timer::Timer()
{
  QueryPerformanceFrequency(&frequency);
}

void Timer::startTimer()
{
  QueryPerformanceCounter(&startTime);
}

void Timer::stopTimer()
{
  QueryPerformanceCounter(&endTime);
}

int Timer::getElapsedTime()
{
  elapsedTimeMicroseconds.QuadPart = endTime.QuadPart - startTime.QuadPart;
  elapsedTimeMicroseconds.QuadPart *= 1000000;
  elapsedTimeMicroseconds.QuadPart /= frequency.QuadPart;
  return elapsedTimeMicroseconds.QuadPart;
}