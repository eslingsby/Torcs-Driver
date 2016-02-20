#include "MyDriver.hpp"

#include <vector>
#include "Renderer.hpp"
#include <glm\vec2.hpp>
#include <algorithm>
#include "Utils.hpp"

const float MyDriver::_speedDefault = 60.f;

const float MyDriver::_speedBoost = 150.f;
const float MyDriver::_speedBrake = 90.f;

const float MyDriver::_forwardDivisor = 100.f;

const int MyDriver::_gearUp[6] = { 5000, 6000, 6000, 6500, 7000, 0 };
const int MyDriver::_gearDown[6] = { 0, 2500, 3000, 3000, 3500, 3500 };

MyDriver::MyDriver(bool log){
	_log = log;
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
		Renderer::get().setGraph(0, 100.f, -0.5f, 0.5f);
		Renderer::get().setGraph(1, 100.f, -0.5f, 0.5f);
		Renderer::get().setGraph(2, 100.f, -0.5f, 0.5f);
	}
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;
	
	CarControl cc;

	//_lineSteering(cs, cc);
	_smartSteering(cs, cc);

	_pedals(cs, cc);
	
	if (_log)
		Renderer::get().setWindowTitle(std::to_string(_dt.count()));

	return cc;
}

void MyDriver::_lineSteering(CarState& cs, CarControl& cc){

	float leftTrack = cs.getTrack(0 + 2);
	float rightTrack = cs.getTrack(TRACK_SENSORS_NUM - 1 - 2);

	_trackOffset = 0.f;

	float total = _trackOffset * (leftTrack + rightTrack);

	float steer = (leftTrack - total / 2) - (rightTrack + total / 2);

	if (_log)
		Renderer::get().drawGraph({ _runtime.count() / 100.f, steer }, 1);


	cc.setSteer(steer);
}

void MyDriver::_pedals(CarState& cs, CarControl& cc){
	float forward = cs.getTrack((TRACK_SENSORS_NUM - 1) / 2) / _forwardDivisor;

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
		cc.setBrake(forward);
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

		float dist = 35.f;

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
			float i_divisor = 100.f;
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

			// P + I + D
			float newAngle = proportional + integral + derivative;

			cc.setSteer(newAngle);

			// Graphing
			if (_log)
				Renderer::get().drawGraph({ _runtime.count() / 100.f, newAngle }, 1);

			// Adding to history
			_turningAngleHistory.push_back(newAngle);

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