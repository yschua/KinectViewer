#pragma once
#include <Windows.h>

namespace Core {

class Timer {
  LARGE_INTEGER startTime;
  LARGE_INTEGER endTime;
  LARGE_INTEGER elapsedTimeMicroseconds;
  LARGE_INTEGER frequency;
public:
  Timer();
  void startTimer();
  void stopTimer();
  float getElapsedTime();
  float getFPS();
};

}