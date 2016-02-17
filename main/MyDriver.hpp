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

	float _speedDefault = 60.f;
	float _speedBoost = 80.f;
	float _speedBrake = 30.f;

	bool _clutching = false;

	int _gearUp = 5000;
	int _gearDown = 3000;

	float _forwardDivisor = 100.f;

	float _trackOffset = 0.f;

	bool _log;

	float getSpeed(CarState& cs);
	void engageGear(CarControl& cc, int gear = 1);
	void steer(CarState& cs, CarControl& cc);
	float obstacle(CarState& cs);

public:
	MyDriver(bool log = false);
	~MyDriver();

	void init(float* angles);
	CarControl wDrive(CarState cs);

	void onRestart();
	void onShutdown();
};