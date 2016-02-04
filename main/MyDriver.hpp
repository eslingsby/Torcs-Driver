#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>
#include <glm\glm.hpp>

class MyDriver : public WrapperBaseDriver{
	CarControl _previousControl;
	CarControl _currentControl;

	CarState _previousState;
	CarState _currentState;

	typedef chrono::high_resolution_clock Clock;

	Clock::time_point _currentTime = Clock::now();
	Clock::time_point _previousTime = Clock::now();

public:
	MyDriver();
	~MyDriver();

	void init(float* angles);

	CarControl wDrive(CarState cs);

	void onRestart();

	void onShutdown();
};