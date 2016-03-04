#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>
#include <vector>

class MyDriver : public WrapperBaseDriver{
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::duration<double, milli> Milliseconds;

	Clock::time_point _timeCurrent = Clock::now();
	Clock::time_point _timePrevious = _timeCurrent;

	Milliseconds _dt = _timeCurrent - _timePrevious;
	Milliseconds _runtime = Milliseconds::zero();

	unsigned int _logging;

	static const float _nonLoggingStunt;

	static const float _trackAngles[19];

	static const int _gearUp[6];
	static const int _gearDown[6];

	static const int _maxBlock;
	static const float _blockDropOff;

	static const float _middleDrift;

	static const float _awarnessTrack;
	static const float _awarnessOpponent;

	static const float _easeSteer;
	static const float _easeBrake;
	static const float _easeAccel;
	
	static const float _p;
	static const float _i;
	static const float _d;

	static const unsigned int _historySteerLength;

	struct{
		float sensors[36];
		float delta[36];

		unsigned int furthestRay = 9;

		float steer = 0.f;
		std::vector<float> steerHistory;
		
		int gear = 1;

		float speed = 0.f;
		float brake = 0.f;

		bool crashed = false;
	}_driving;

public:
	MyDriver(unsigned int logging = 0);

	void onRestart();
	void onShutdown();

	void init(float* angles = nullptr);
	CarControl wDrive(CarState cs);
};