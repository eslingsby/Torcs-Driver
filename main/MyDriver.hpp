#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>

class MyDriver : public WrapperBaseDriver{
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::duration<double, milli> Milliseconds;

	Clock::time_point _timeCurrent = Clock::now();
	Clock::time_point _timePrevious = _timeCurrent;

	Milliseconds _dt = _timeCurrent - _timePrevious;
	Milliseconds _runtime = Milliseconds::zero();

	const bool _log;

	static const int _gearUp[6];
	static const int _gearDown[6];

	static const int _maxBlock;

	static const float _middleDrift;

	static const float _awarnessTrack;
	static const float _awarnessOpponent;

	static const float _easeSteer;
	static const float _easeBrake;
	static const float _easeAccel;

	struct{
		float steer = 0.f;

		int gear = 1;

		float speed = 0.f;
		float brake = 0.f;
		float boostTime = 20000;

		float sensors[36];
	}_driving;

public:
	MyDriver(bool log = false);

	void onRestart();
	void onShutdown();

	void init(float* angles = nullptr);
	CarControl wDrive(CarState cs);
};