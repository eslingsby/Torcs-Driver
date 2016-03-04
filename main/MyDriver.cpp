#include "MyDriver.hpp"

#include "Renderer.hpp"
#include "Utils.hpp"
#include <glm\gtc\constants.hpp>

#define ALL_SENSORS 36
#define HALF_SENSORS 19
#define QUART_SENSORS 9

// Stunt divisor for drivers not being logged, 1.f = un-stunted
const float MyDriver::_nonLoggingStunt = 0.75f;

// Track sensors
const float MyDriver::_trackAngles[HALF_SENSORS] = { -90, -80, -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };

// Gear RPM changes from SimpleDriver
const int MyDriver::_gearUp[6] = { 5000, 6000, 6000, 6500, 7000, 0 }; //{ 5000, 6000, 6000, 6500, 7000, 0 };
const int MyDriver::_gearDown[6] = { 0, 2500, 3000, 3000, 3500, 3500 }; //{ 0, 2000, 3000, 3000, 3500, 3500 };

// 0 = doesn't care, 1 = stays in the middle
const float MyDriver::_middleDrift = 0.f; //0.001f;

// 0 = no awarness, 1 = full awarness
const float MyDriver::_awarnessTrack = 0.8f;
const float MyDriver::_awarnessOpponent = 0.135f; // 0.135f;

// Sensors a single opponent can block
const int MyDriver::_maxBlock = 6;

// The higher the more rounded the edge of a blocking opponent is
const float MyDriver::_blockDropOff = 10.f;

// 0 = full ease, 1 = no ease
const float MyDriver::_easeSteer = 1.f; //0.5f;
const float MyDriver::_easeBrake = 0.1f;
const float MyDriver::_easeAccel = 0.1f; //0.05f;

const unsigned int MyDriver::_historySteerLength = 8;

const float MyDriver::_p = 0.25f;
const float MyDriver::_i = 0.075f;//0.1f;
const float MyDriver::_d = 0.7f;

//const float MyDriver::_p = 1.f;
//const float MyDriver::_i = 0.f;
//const float MyDriver::_d = 0.f;

MyDriver::MyDriver(unsigned int logging) : _logging(logging){}

void MyDriver::onRestart(){
	init();
}

void MyDriver::onShutdown(){
	init();
}

void MyDriver::init(float* angles){
	if (angles != nullptr){
		for (int i = 0; i < HALF_SENSORS; i++)
			angles[i] = _trackAngles[i];
	}

	_timeCurrent = Clock::now();
	_timePrevious = _timeCurrent;

	_dt = _timeCurrent - _timePrevious;

	_runtime = Milliseconds::zero();

	std::fill_n(_driving.sensors, ALL_SENSORS, 200.f);
	std::fill_n(_driving.delta, ALL_SENSORS, 0.f);

	if (_logging){
		Renderer::get().setGraph(0, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(1, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(2, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(3, 100.f, -0.5f, 0.5f, true);
		Renderer::get().setGraph(4, 100.f, 0.f, 100.f, true);
	}
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;

	if (_logging)
		Renderer::get().setWindowTitle(std::to_string(_dt.count()));

	// Copy old sensors to delta and reset sensors with max distance
	memcpy(&_driving.delta[0], &_driving.sensors[0], sizeof(_driving.sensors));
	std::fill_n(_driving.sensors, ALL_SENSORS, 200.f);
	

	// Directly mapping track sensors to front 19 sensors
	int zero = 0;

	for (int i = 0; i < HALF_SENSORS; i++){
		float distance = cs.getTrack(i);

		if (distance <= 0.f)
			zero++;

		if (distance < _driving.sensors[i + QUART_SENSORS])
			_driving.sensors[i + QUART_SENSORS] = distance;
	}

	if (zero == 19)
		_driving.crashed = true;
	else if (_driving.crashed)
		_driving.crashed = false;
	
	// Iterate through opponent sensors
	for (int i = 0; i < ALL_SENSORS; i++){
		//std::cout << "i - " << i << "\n";

		float distance = cs.getOpponents(i);

		// If no opponent, skip sensor
		if (distance >= _awarnessOpponent * 200.f)
			continue;

		// Calculate amount of track sensors to block, and round to nearest even integer
		float raw = distance / (_awarnessOpponent * 200.f);

		float block = ((1.f - raw) * (float)((_maxBlock - 2) + 2));
		
		block = glm::roundEven(block);

		// Apply smaller distances to sensor array
		for (int x = block / 2; x >= -(block / 2 - 1); x--){
			int sensor = (i + x) % ALL_SENSORS;
		
			float dropOff = glm::abs(changeRange(block / 2, -(block / 2 - 1), -1.f, 1.f, x));

			if (_driving.sensors[sensor] > distance + dropOff * _blockDropOff)
				_driving.sensors[sensor] = distance + dropOff * _blockDropOff;
		}
	}
	

	// Calculate difference using old sensors stored in delta
	for (int i = 0; i < ALL_SENSORS; i++){
		_driving.delta[i] = _driving.delta[i] - _driving.sensors[i];

		if (_driving.delta[i] < 0.f)
			_driving.delta[i] = glm::abs(_driving.delta[i]);
		else
			_driving.delta[i] = 0.f;

		if (_driving.delta[i] > 0.25f)
			_driving.delta[i] = 0.f;
	}


	// Adding boost to previous furthest ray
	if (_driving.furthestRay <= QUART_SENSORS){
		for (int i = _driving.furthestRay; i < HALF_SENSORS; i++){
			_driving.sensors[QUART_SENSORS + i] *= changeRange(_driving.furthestRay, 18, 1, 0, i);
		}
	}
	
	if (_driving.furthestRay >= QUART_SENSORS){
		for (int i = _driving.furthestRay; i >= 0; i--){
			_driving.sensors[QUART_SENSORS + i] *= changeRange(_driving.furthestRay, 0, 1, 0, i);
		}
	}

	
	// Choosing which ray to steer towards, and how fast
	float maxDistance = _driving.sensors[_driving.furthestRay + QUART_SENSORS];

	for (int i = 0; i < HALF_SENSORS; i++){
		float distance = _driving.sensors[i + QUART_SENSORS];

		if (maxDistance < distance){
			maxDistance = distance;
			_driving.furthestRay = i;
		}
	}


	// P - PID
	float proportional = (changeRange(0.f, 18.f, 1.5f, -1.5f, _driving.furthestRay) - (cs.getTrackPos() / 500.f)) - cs.getTrackPos() * _middleDrift;

	proportional *= _p;


	// I - PID
	while (_driving.steerHistory.size() > _historySteerLength)
		_driving.steerHistory.erase(_driving.steerHistory.begin());

	float integral = 0.f;

	for (float i : _driving.steerHistory)
		integral += i;

	integral *= _i;


	// D - PID
	float derivative = 0.f;

	if (_driving.steerHistory.size() >= 2)
		derivative = _driving.steerHistory[_driving.steerHistory.size() - 1] - _driving.steerHistory[_driving.steerHistory.size() - 2];

	derivative *= _d;


	// P + I + D
	float oldSteer = _driving.steer;

	_driving.steer = proportional + integral + derivative;

	_driving.steerHistory.push_back(_driving.steer);

	_driving.steer = glm::mix(oldSteer, _driving.steer, _easeSteer);


	// Speed control
	if (!_driving.crashed){
		float speed = _driving.sensors[_driving.furthestRay + QUART_SENSORS] / (200.f * _awarnessTrack) + _driving.delta[_driving.furthestRay + QUART_SENSORS] * 4.f;

		if (!_logging)
			speed *= _nonLoggingStunt;

		_driving.speed = glm::mix(_driving.speed, speed, _easeAccel); //*(1.f - cs.getTrack(QUART_SENSORS)) * brake;

		float brake = (cs.getSpeedX() / 200.f) - _driving.speed;

		if (brake > 0.f)
			_driving.brake = glm::mix(_driving.brake, brake, _easeBrake);
		else
			_driving.brake = 0.f;
	}


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
	

	// If logging, render sensors
	if (_logging){
		//Renderer::get().drawGraph({ _runtime.count() / 100.f, proportional }, 0);
		//Renderer::get().drawGraph({ _runtime.count() / 100.f, integral }, 1);
		//Renderer::get().drawGraph({ _runtime.count() / 100.f, derivative }, 2);
		Renderer::get().drawGraph({ _runtime.count() / 100.f, _driving.steer }, 3);

		for (int i = 0; i < ALL_SENSORS; i++){
			float distance = _driving.sensors[i];

			if (distance < 0.1f)
				continue;
		
			float radians = glm::radians(i * 10.f);
		
			glm::vec2 point = { -glm::sin(radians), -glm::cos(radians) };
		
			glm::vec3 colour = { 1.f, 1.f, 1.f };
		
			if (_driving.furthestRay + QUART_SENSORS == i)
				colour = { 0.f, 1.f, 0.f };

			float zoom = 10.f;

			Renderer::get().drawLine({ 0, 0 }, point * distance * zoom, colour);
		}

		// Cool effects
		//Renderer::get().setRotation(cs.getAngle() * 90.f);
		//Renderer::get().setZoom(1.f + cs.getSpeedX() / 200.f);

		if (_driving.crashed)
			std::cout << "Crashed!\n";
	}


	// Applying to controller
	CarControl cc;

	cc.setGear(_driving.gear);
	cc.setAccel(_driving.speed);
	cc.setSteer(_driving.steer);
	cc.setBrake(_driving.brake);

	return cc;
}