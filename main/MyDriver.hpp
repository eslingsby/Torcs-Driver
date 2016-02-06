#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>
#include <glm\glm.hpp>

class MyDriver : public WrapperBaseDriver{
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::duration<double, milli> Milliseconds;

	Clock::time_point _timeCurrent = Clock::now();
	Clock::time_point _timePrevious = _timeCurrent;

	Milliseconds _dt = _timeCurrent - _timePrevious;
	Milliseconds _runtime = Milliseconds::zero();

	bool _clutching = false;
	float _maxSpeed = 40.f;

	float getSpeed(CarState& cs);
	void engageGear(CarControl& cc, int gear = 1);
	float steer(CarState& cs, CarControl& cc);

public:
	MyDriver();
	~MyDriver();

	void init(float* angles);
	CarControl wDrive(CarState cs);

	void onRestart();
	void onShutdown();
};