#pragma once

#include <WrapperBaseDriver.h>
#include <chrono>
#include <glm\glm.hpp>
#include <vector>

class OldDriver : public WrapperBaseDriver{
	typedef chrono::high_resolution_clock Clock;
	typedef chrono::duration<double, milli> Milliseconds;

	Clock::time_point _timeCurrent = Clock::now();
	Clock::time_point _timePrevious = _timeCurrent;

	Milliseconds _dt = _timeCurrent - _timePrevious;
	Milliseconds _runtime = Milliseconds::zero();

	float OldDriver::_speedDefault;
	float OldDriver::_speedBoost;
	float OldDriver::_speedBrake;


	//static const float _speedDefault;
	//
	//static const float _speedBoost;
	//static const float _speedBrake;

	static const float _forwardDivisor;

	static const int _gearUp[6];
	static const int _gearDown[6];

	static const float _obstacleDivisor;
	static const float _obstacleBrake;

	static const float _forwardDistance;
	static const float _steerDistance;

	static const unsigned int _forwardSpan;
	static const unsigned int _sideSpan;

	static const float _speedToTurn;

	static const float _oppenentLine;

	bool _clutching = false;
	float _trackOffset = 0.f;
	glm::vec2 _steerPush = { 0.f, 0.f };
	float _obstacleAhead = 0.f;

	std::vector<float> _turningAngleHistory;
	std::vector<glm::vec2> _steerPushHistory;

	bool _log;

	void _steerClamps(CarState& cs);
	float _checkObstacles(CarState& cs, unsigned int start, unsigned end, float distance);
	void _lineSteering(CarState& cs, CarControl& cc);
	void _smartSteering(CarState& cs, CarControl& cc);
	void _pedals(CarState& cs, CarControl& cc);
	void _engageGear(CarControl& cc, int gear = 1);

public:
	OldDriver(bool log = false);
	~OldDriver();

	void onRestart();
	void onShutdown();

	void init(float* angles);
	CarControl wDrive(CarState cs);
};