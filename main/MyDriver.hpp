#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>
#include <glm\glm.hpp>
#include <vector>

class MyDriver : public WrapperBaseDriver{
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::duration<double, milli> Milliseconds;

	Clock::time_point _timeCurrent = Clock::now();
	Clock::time_point _timePrevious = _timeCurrent;

	Milliseconds _dt = _timeCurrent - _timePrevious;
	Milliseconds _runtime = Milliseconds::zero();

	static const float _speedDefault;
	static const float _speedBoost;
	static const float _speedBrake;
	static const int _gearUp[6];
	static const int _gearDown[6];

	static const float _forwardDivisor;

	bool _clutching = false;
	float _trackOffset = 0.f;

	std::vector<float> _turningAngleHistory;

	bool _log;	

	void _lineSteering(CarState& cs, CarControl& cc);
	void _smartSteering(CarState& cs, CarControl& cc);
	void _pedals(CarState& cs, CarControl& cc);

	void _engageGear(CarControl& cc, int gear = 1);

public:
	MyDriver(bool log = false);
	~MyDriver();

	void onRestart();
	void onShutdown();

	void init(float* angles);
	CarControl wDrive(CarState cs);
};