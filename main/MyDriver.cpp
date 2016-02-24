#include "MyDriver.hpp"

#include <vector>
#include "Renderer.hpp"
#include <glm\vec2.hpp>
#include <algorithm>
#include "Utils.hpp"



float randomOffset(float range){
	return -(rand() % (int)range);
}

//const float MyDriver::_speedDefault = 60.f;
//
//const float MyDriver::_speedBoost = 150.f;
//const float MyDriver::_speedBrake = 90.f;

const float MyDriver::_forwardDivisor = 100.f;

const int MyDriver::_gearUp[6] = { 5000, 6000, 6000, 6500, 7000, 0 };
const int MyDriver::_gearDown[6] = { 0, 2500, 3000, 3000, 3500, 3500 };

const float MyDriver::_obstacleDistance = 6.f;
const float MyDriver::_obstacleDivisor = 0.25f;
const float MyDriver::_obstacleBrake = 2.f;

MyDriver::MyDriver(bool log){
	_log = log;


	_speedDefault = 60.f;

	_speedBoost = 150.f;

	_speedBrake = 90.f;


	if (!_log){
		_speedDefault += randomOffset(80.f);
		//_speedBoost += randomOffset(80.f);
		//_speedBrake += randomOffset(80.f);
	}
}

MyDriver::~MyDriver(){
	
}

void MyDriver::onRestart(){
	init(nullptr);
}

void MyDriver::onShutdown(){
	init(nullptr);
}

void MyDriver::init(float* angles){
	// -90, -80, -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90

	if (angles != nullptr){
		for (int i = 0; i < 19; i++){
			angles[i] = -90 + (10 * i);
		}
	}

	if (_log){
		Renderer::get().setGraph(0, 100.f, -1.f, 1.f, true);
		Renderer::get().setGraph(1, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(2, 100.f, -1.f, 1.f, true);
	}
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;
	
	CarControl cc;

	_steerClamps(cs);

	//_lineSteering(cs, cc);
	_smartSteering(cs, cc);

	_pedals(cs, cc);
	
	if (_log)
		Renderer::get().setWindowTitle(std::to_string(_dt.count()));

	return cc;
}

float MyDriver::_checkObstacles(CarState& cs, unsigned int start, unsigned end, float distance){
	float angle = (start + end) / 2.f;
	float minDistance = 200.f;

	for (unsigned int i = start; i <= end; i++){
		if (minDistance > cs.getOpponents(i)){
			minDistance = cs.getOpponents(i);
			angle = (float)i;
		}
	}

	angle = changeRange(0.f, 36.f, -1.f, 1.f, angle);


	if (minDistance <= distance)
		minDistance = 1.f - changeRange(4.f, distance, 0.f, 1.f, minDistance);
	else
		minDistance = 0.f;

	return minDistance;
}

void MyDriver::_steerClamps(CarState& cs){
	// Updates _obstacleAhead and _steerClamp

	unsigned int middle = 36 / 2;

	_obstacleAhead = _checkObstacles(cs, middle - 1, middle, 30.f);

	//glm::vec2 forwardLeft = _checkObstacles(cs, middle - 6, middle - 3);
	//glm::vec2 forwardRight = _checkObstacles(cs, middle + 2, middle + 5);
	//glm::vec2 directLeft = _checkObstacles(cs, middle - 9, middle - 7);
	//glm::vec2 directRight = _checkObstacles(cs, middle + 6, middle + 8);

	//int start = 0;
	//int end = 5;

	float distance = 15.f;

	float forwardLeft = -_checkObstacles(cs, middle - 9, middle - 1, distance) / (cs.getSpeedX() / 10.f);
	float forwardRight = _checkObstacles(cs, middle, middle + 8, distance) / (cs.getSpeedX() / 10.f);

	//_steerPush.x = -(1.f - forwardLeft.y) / _obstacleDivisor;
	//_steerPush.y = (1.f - forwardRight.y) / _obstacleDivisor;




	//_steerPush.x = glm::lerp(_steerPush.x, -(forwardLeft / _obstacleDivisor), 0.5f);
	//_steerPush.y = (forwardRight / _obstacleDivisor);




	_steerPush.x = forwardLeft / _obstacleDivisor;
	_steerPush.y = forwardRight / _obstacleDivisor;


	//std::cout << cs.getTrackPos() * -1 << "\n";

	if (cs.getTrackPos() >= -0.8f)
		//_steerPush.x


	//_steerClamp.x = -(1.f - directLeft.y);
	//_steerClamp.y = 1.f - directRight.y;

	//_steerClamp

	//_steerClamp.x = glm::clamp(_steerClamp.x, -1.f, 0.f);
	//_steerClamp.y = glm::clamp(_steerClamp.y, 0.f, 1.f);

	if (_log){
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _steerPush.x }, 0);
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _steerPush.y }, 2);
	}
	//std::cout << _steerClamp.x << " - " << _steerClamp.y << "\n";
}

void MyDriver::_lineSteering(CarState& cs, CarControl& cc){

	float leftTrack = cs.getTrack(0 + 2);
	float rightTrack = cs.getTrack(TRACK_SENSORS_NUM - 1 - 2);

	_trackOffset = 0.f;

	float total = _trackOffset * (leftTrack + rightTrack);

	float steer = (leftTrack - total / 2) - (rightTrack + total / 2);

	if (_log)
		Renderer::get().drawGraph({ _runtime.count() / 100.f, steer }, 1);

	// Todo: take into account _steerClamp

	cc.setSteer(steer);
}

void MyDriver::_pedals(CarState& cs, CarControl& cc){
	float forward = (cs.getTrack((TRACK_SENSORS_NUM - 1) / 2) / _forwardDivisor) * (1.f - _obstacleAhead * _obstacleBrake);

	// Todo: take into account _obstacleAhead

	int gear = cs.getGear();
	int rpm = cs.getRpm();

	if (_runtime.count() > 5000.f)
		cc.setAccel(1.f);

	if (cs.getSpeedX() < _speedDefault - (forward * _speedBrake) + (forward * _speedBoost)){
		if (gear == 0)
			_engageGear(cc);
		else if (rpm >= _gearUp[gear - 1] && gear != 6)
			_engageGear(cc, gear + 1);


		if (!_clutching)
			cc.setAccel(1.f);
	}
	else{
		if (rpm <= _gearDown[gear - 1] && gear != 2)
			_engageGear(cc, gear - 1);

		cc.setAccel(0.f);
		cc.setBrake(forward + _obstacleAhead * _obstacleBrake);
	}
}

void MyDriver::_engageGear(CarControl& cc, int gear){
	if (!_clutching){
		_clutching = true;
		cc.setClutch(1.f);
	}
	else{
		if (cc.getGear() == gear){
			_clutching = false;
			cc.setClutch(0.f);
			return;
		}

		cc.setGear(gear);
	}
}

void MyDriver::_smartSteering(CarState& cs, CarControl& cc){
	float track[36];
	float car[36];

	// Lerping track sensors into 36 equal sensors
	for (int i = 0; i < 36; i++){
		float x = changeRange(0, 35, 0, 18, i);

		int floor = (int)glm::floor(x);
		int ceil = (int)glm::ceil(x);

		float tail = x - floor;

		float value = lerp(cs.getTrack(floor), cs.getTrack(ceil), tail);

		track[i] = value;
		car[i] = cs.getOpponents(i);
	}

	// Rendering values
	float size = 3.f;
	glm::vec2 offset = { 0.f, -256.f };

	// For each sensors in track and car sensors
	for (int i = 0; i < 36; i++){
		float angle = glm::radians(changeRange(0, 35, 360, 0, i)) - glm::radians(90.f);

		glm::vec3 colour = { 0.75f, 0.75f, 0.75f };

		float dist = 25.f;

		if (car[i] < dist)
			colour = { colour.r, car[i] / (dist / colour.g), car[i] / (dist / colour.b) };

		int maxIndex = 0;
		float maxCount = track[0] - (dist - car[0] / (200.f / dist));

		for (int x = 0; x < 36; x++){
			float value;

			if (car[x] < dist)
				value = track[x] - (dist - car[x] / (200.f / dist));
			else
				value = track[x];

			if (value > maxCount){
				maxIndex = x;
				maxCount = value;
			}
		}

		if (i == maxIndex){
			float p_divisor = 1.f;
			float i_divisor = 75.f;
			float d_divisor = 1.f;

			unsigned int i_length = 16;
			unsigned int d_gap = 2;

			// P
			float proportional = (maxIndex * 10.f) + 5.f;

			proportional = -changeRange(5.f, 355.f, -1.f, 1.f, proportional);

			proportional /= p_divisor;

			// I
			float integral = 0.f;

			for (float i : _turningAngleHistory)
				integral += i;

			integral /= i_divisor;

			// D
			float derivative = 0.f;

			if (_turningAngleHistory.size() >= d_gap)
				derivative = _turningAngleHistory[_turningAngleHistory.size() - 1] - _turningAngleHistory[_turningAngleHistory.size() - (d_gap - 1)];

			derivative /= d_divisor;

			// clamp(P + I + D, min, max)
			float loggedAngle = proportional;

			float newAngle = loggedAngle;

			//newAngle = glm::clamp(newAngle, _steerClamp.x, _steerClamp.y);

			//newAngle 

			//newAngle += (1.f - _steerPush.x) / 10.f;
			//newAngle -= (1.f + _steerPush.y) / 10.f;

			newAngle += _steerPush.x / 10.f;
			newAngle += _steerPush.y / 10.f;

			//newAngle += 1.f - _steerClamp.x;




			// Takes into account _steerClamp
			cc.setSteer(newAngle);

			// Graphing
			if (_log)
				Renderer::get().drawGraph({ _runtime.count() / 100.f, loggedAngle }, 1);

			// Adding to history
			_turningAngleHistory.push_back(loggedAngle);

			if (_turningAngleHistory.size() > i_length)
				_turningAngleHistory.erase(_turningAngleHistory.begin());

			colour = { 0.f, 1.f, 0.f };
		}

		if (_log){
			Renderer::get().drawLine(
					offset,
					{ track[i] * glm::cos(angle) * size + offset.x, track[i] * glm::sin(angle) * size + offset.y },
					colour
				);

			if (car[i] >= dist)
				continue;
			
			Renderer::get().drawPoint(
				{ car[i] * glm::cos(angle) * size + offset.x, car[i] * glm::sin(angle) * size + offset.y },
				colour
			);
		}
	}
}