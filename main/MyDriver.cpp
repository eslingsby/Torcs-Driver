#include "MyDriver.hpp"

#include "Renderer.hpp"
#include "Utils.hpp"
#include <glm\gtc\constants.hpp>

// Gear RPM changes from SimpleDriver
//const int MyDriver::_gearUp[6] = { 5000, 6000, 6000, 6500, 7000, 0 };
//const int MyDriver::_gearDown[6] = { 0, 2000, 3000, 3000, 3500, 3500 };
const int MyDriver::_gearUp[6] = { 5000, 6000, 6000, 6500, 7000, 0 };
const int MyDriver::_gearDown[6] = { 0, 2500, 3000, 3000, 3500, 3500 };

// Max track sensors a single opponent can block
const int MyDriver::_maxBlock = 6;

const float MyDriver::_middleDrift = 0.001f; //0.005f;

const float MyDriver::_awarnessTrack = 0.7f;
const float MyDriver::_awarnessOpponent = 0.125f;

const float MyDriver::_easeSteer = 0.5f; //0.5f;
const float MyDriver::_easeBrake = 0.1f;
const float MyDriver::_easeAccel = 0.1f; //0.05f;

MyDriver::MyDriver(bool log) : _log(log){}

void MyDriver::onRestart(){
	init();
}

void MyDriver::onShutdown(){
	init();
}

void MyDriver::init(float* angles){
	float newAngles[36] = { -90, -80, -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };

	if (angles != nullptr){
		for (int i = 0; i < 19; i++)
			angles[i] = newAngles[i];
	}

	_runtime = Milliseconds::zero();

	std::fill_n(_driving.sensors, 36, 200.f);

	if (_log){
		Renderer::get().setGraph(0, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(1, 100.f, 0.f, 2.f, true);
		Renderer::get().setGraph(2, 100.f, 0.f, 1.f, true);
	}
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;

	if (_log)
		Renderer::get().setWindowTitle(std::to_string(_dt.count()));
	
	// Reset sensors with max distance
	std::fill_n(_driving.sensors, 36, 200.f);

	
	// Clamping sensors to an egg
	/*for (int i = 0; i < 36; i++){
		float radians = glm::radians(i * 10.f);
	
		glm::vec2 ray;
	
		float egg = 8.f;
	
		//if ((i >= 0 && i < 9) || i > 27)
		ray = { -glm::sin(radians), -glm::cos(radians) };
		//else
		//	ray = { -glm::sin(radians) / egg, -glm::cos(radians) };
	
		//-glm::sin(radians);
	
		//sensors[i] *= glm::length(ray) * 200.f;
	
		Renderer::get().drawLine({ 0, 0 }, ray, { 1.f, 0.f, 0.f });
	}*/

	
	// Lerping track sensors into 36 equal sensors
	/*for (int i = 0; i < 36; i++){
		float x = changeRange(0, 35, 0, 18, i);
	
		int floor = (int)glm::floor(x);
		int ceil = (int)glm::ceil(x);
	
		float tail = x - floor;
	
		float distance = lerp(cs.getTrack(floor), cs.getTrack(ceil), tail);
	
		if (glm::length(distance) < sensors[i])
			sensors[i] = distance;
	}*/
	

	// Directly mapping track sensors to front 19 sensors
	for (int i = 0; i < 19; i++){
		float distance = cs.getTrack(i);

		if (distance < _driving.sensors[i + 9])
			_driving.sensors[i + 9] = distance;
	}

	
	// Iterate through opponent sensors
	for (int i = 0; i < 36; i++){
		//std::cout << "i - " << i << "\n";

		float distance = cs.getOpponents(i);

		// If no opponent, skip sensor
		if (distance >= _awarnessOpponent * 200.f)
			continue;

		// Calculate amount of track sensors to block, and round to nearest even integer
		//float raw = glm::tan((distance / 100.f) * glm::half_pi<float>());
		//float raw = distance / 50.f; //(before _awarnessOpponent)
		//float raw = distance / ((_awarnessOpponent * 200.f) * 2.f);
		float raw = distance / (_awarnessOpponent * 200.f);

		float block = ((1.f - raw) * (float)((_maxBlock - 2) + 2));
		
		block = glm::roundEven(block);

		// Apply smaller distances to sensor array
		for (int x = block / 2; x >= -(block / 2 - 1); x--){
			//std::cout << "x - " << x << "\n";

			int sensor = (i + x) % 36;
		
			if (_driving.sensors[sensor] > distance)
				_driving.sensors[sensor] = distance;
		}
	}

	
	// Choosing which ray to steer towards, and how fast
	int maxSensor = 9;
	float maxDistance = _driving.sensors[maxSensor + 9];

	for (int i = 0; i < 19; i++){
		float distance = _driving.sensors[i + 9];

		if (maxDistance < distance){
			maxDistance = distance;
			maxSensor = i;
		}
	}

	float steer = (changeRange(0.f, 18.f, 1.5f, -1.5f, maxSensor) - (cs.getTrackPos() / 500.f)) - cs.getTrackPos() * _middleDrift;

	_driving.steer = glm::mix(_driving.steer, steer, _easeSteer);
	//_driving.steer = newSteer;


	// Speed control
	float speed = maxDistance / (200.f * _awarnessTrack);

	_driving.speed = glm::mix(_driving.speed, speed, _easeAccel); //*(1.f - cs.getTrack(9)) * brake;

	float brake = (cs.getSpeedX() / 200.f) - _driving.speed;

	if (brake > 0.f)
		_driving.brake = glm::mix(_driving.brake, brake, _easeBrake);
	else
		_driving.brake = 0.f;

	// Changing gears
	int gear = cs.getGear();
	int rpm = cs.getRpm();

	if (gear < 1)
		_driving.gear = 1;

	if (gear < 6 && rpm >= _gearUp[gear - 1])
		_driving.gear = gear + 1;
	else
		if (gear > 1 && rpm <= _gearDown[gear - 1])
			_driving.gear = gear - 1;


	// Applying to controller
	CarControl cc;

	cc.setGear(_driving.gear);
	cc.setAccel(_driving.speed);
	cc.setSteer(_driving.steer);
	cc.setBrake(_driving.brake);


	// If logging, render sensors
	if (_log){
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _driving.steer }, 0);
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _driving.speed }, 1);
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _driving.brake }, 2);

		for (int i = 0; i < 36; i++){
			float distance = _driving.sensors[i];

			float radians = glm::radians(i * 10.f);

			glm::vec2 point = { -glm::sin(radians), -glm::cos(radians) };

			glm::vec3 colour = { 1.f, 1.f, 1.f };

			if (maxSensor + 9 == i)
				colour = { 0.f, 1.f, 0.f };

			Renderer::get().drawLine({ 0, 0 }, point * distance * 5.f, colour);
		}
		
		// Cool effects
		Renderer::get().setRotation(cs.getAngle() * 90.f);
		Renderer::get().setZoom(1.f + cs.getSpeedX() / 200.f);
	}

	return cc;
}