#pragma once
#include <chrono>
#include "EngineAPI.hpp"

class ENGINE_API Timer {
  public:
    Timer();
    float Mark();
    float Peek() const;

  private:
    std::chrono::steady_clock::time_point last;
};
