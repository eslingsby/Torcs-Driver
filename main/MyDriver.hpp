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

	bool _logging;
	float _speedRestrict;

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
		float sensors[36]; // Culmination of 36 opponent sensors and 19 track sensors
		float delta[36]; // Difference between this state's sensors and previous state's

		unsigned int furthestRay = 9; // The sensor which the driver will steer towards

		float steer = 0.f; // Steering angle target
		std::vector<float> steerHistory; // Steering angle history
		
		int gear = 1; // Gear target

		float speed = 0.f; // Speed target
		float brake = 0.f; // Brake target

		bool crashed = false; // If the driver has crashed or not
	}_driving;

public:
	MyDriver(bool logging = false, float speedRestrict = 0.f);

	void onRestart();
	void onShutdown();

	void init(float* angles = nullptr);
	CarControl wDrive(CarState cs);
};