#pragma once
#include <chrono>

class Timer
{
public:
	Timer();
	float Mark();
	float Peak() const;
private:
	std::chrono::steady_clock::time_point last;
};