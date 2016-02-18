#include "MyDriver.hpp"

#include <vector>
#include "Renderer.hpp"
#include <glm\vec2.hpp>
#include <algorithm>
#include "Utils.hpp"

MyDriver::MyDriver(bool log){
	_log = log;
}

MyDriver::~MyDriver(){
	
}

void MyDriver::init(float* angles){
	// -90, -80, -70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90

	for (int i = 0; i < 19; i++){
		angles[i] = -90 + (10 * i);
	}

	if (_log){
		Renderer::get().setGraph(0, 100.f, 0, 6.f);
		Renderer::get().setGraph(1, 100.f, 0.f, 200.f);
	}
	//	Renderer::get().init();


}

float MyDriver::obstacle(CarState& cs){
	//changeRange(-90, 90, 0, 19, )

	float track[36];
	float car[36];

	for (int i = 0; i < 36; i++){
		float x = changeRange(0, 35, 0, 18, i);
	
		int floor = (int)glm::floor(x);
		int ceil = (int)glm::ceil(x);

		float tail = x - floor;

		float value = lerp(cs.getTrack(floor), cs.getTrack(ceil), tail);

		track[i] = value;
		car[i] = cs.getOpponents(i);
	}

	float size = 4.f;
	glm::vec2 offset = { 0.f, -128.f };

	for (int i = 0; i < 36; i++){

		float angle = glm::radians(changeRange(0, 35, 360, 0, i)) - glm::radians(90.f);


		glm::vec3 colour = { 1.f, 1.f, 1.f };

		float dist = 35.f;

		if (car[i] < dist)
			colour = { 1.f, car[i] / dist, car[i] / dist };



		int maxIndex = 0;
		float maxCount = track[0] - (dist - car[0] / (200.f / dist));

		for (int x = 0; x < 36; x++){

			float value;

			if (car[x] >= dist)
				value = track[x];
			else
				value = track[x] - (dist - car[x] / (200.f / dist));

			if (value > maxCount){
				maxIndex = x;
				maxCount = value;
			}
		}

		if (i == maxIndex){
			colour = { 0.f, 1.f, 0.f };


			float proportional = changeRange(0.f, 320.f, 0, 360.f, ((maxIndex + 36 / 2) % 36) * 100.f);

			

			//float integral = 0.f;
			//
			//for (float i : _turningAngleHistory)
			//	integral += i;
			//
			//
			//
			//
			//float derivative = 0.f;
			//
			//int end = _turningAngleHistory.size() - 1;
			//
			//if (end > 0)
			//	derivative = _turningAngleHistory[end - 1] - _turningAngleHistory[end];


			float x = float(_runtime.count() / 100.0);

			//Renderer::get().drawGraph({ x, glm::sin(glm::radians(x * 10.f)) }, 0);



			_turningAngle = proportional;


			_turningAngleHistory.push_back(_turningAngle);

			if (_turningAngleHistory.size() > _turningAngleN)
				_turningAngleHistory.erase(_turningAngleHistory.begin());
					


			//std::cout << _turningAngle << "\n";
		}

		


		Renderer::get().drawLine(
			offset,
			{ track[i] * glm::cos(angle) * size + offset.x, track[i] * glm::sin(angle) * size + offset.y },
			colour
		);

		if (car[i] >= dist)
			continue;

		Renderer::get().drawPoint(
			{ car[i] * glm::cos(angle) * size + offset.x, car[i] * glm::sin(angle) * size + offset.y },
			{ 1.f - car[i] / dist, 0.f, 0.f }
		);
	}

	return 0.f;
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;

	//std::cout << _dt.count() << "\n";

	if (_log){
		//Renderer::get().drawGraph({ float(_runtime.count() / 1000.0), cs.getGear() }, 0);
		Renderer::get().drawGraph({ float(_runtime.count() / 1000.0), cs.getSpeedX() }, 1);
	}
	
	CarControl cc;

	steer(cs, cc);

	float forward = cs.getTrack((TRACK_SENSORS_NUM - 1) / 2) / _forwardDivisor;

	if (_runtime.count() > 5000.f)
		cc.setAccel(1.f);

	if (getSpeed(cs) < _speedDefault - (forward * _speedBrake) + (forward * _speedBoost)){
		if (cs.getGear() == 0)
			engageGear(cc);
		else if (cs.getRpm() > _gearUp && cs.getGear() != 6)
			engageGear(cc, cs.getGear() + 1);


		if (!_clutching)
			cc.setAccel(1.f);
	}
	else{
		if (cs.getRpm() < _gearDown && cs.getGear() != 2)
			engageGear(cc, cs.getGear() - 1);

		cc.setAccel(0.f);
		cc.setBrake(forward);
	}



	if (_log){
		Renderer::get().setWindowTitle(std::to_string(_dt.count()));

		obstacle(cs);		
		
		//Renderer::get().update();
	}



	return cc;
}

float MyDriver::getSpeed(CarState& cs){
	glm::vec3 vector = glm::vec3(cs.getSpeedX(), cs.getSpeedY(), cs.getSpeedZ());

	return glm::length(vector);
}

void MyDriver::engageGear(CarControl& cc, int gear){
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

void MyDriver::steer(CarState& cs, CarControl& cc){


	// Get 








	float leftTrack = cs.getTrack(0 + 2);
	float rightTrack = cs.getTrack(TRACK_SENSORS_NUM - 1 - 2);

	////float _trackOffset = 0.f;
	//
	//float carsRight = cs.getOpponents(1);
	//float carsLeft = cs.getOpponents(36 - 2);
	//
	//_trackOffset = (carsRight - carsLeft) / 200.f;
	

	//float rightThreat = 0.f;
	//float leftThreat = 0.f;
	//
	//for (int i = 0; i < 6; i++){
	//	rightThreat += cs.getOpponents(i);
	//	leftThreat += cs.getOpponents((36 - 1) - i);
	//}

	//std::cout << leftThreat << " - " << rightThreat << "\n";





	//float offset = (section + 0.5f) / 5.5f;

	float leftThreat = ((200.f - cs.getOpponents(17)) / 200.f) + ((200.f - cs.getOpponents(16)) / 200.f) + ((200.f - cs.getOpponents(15)) / 200.f);

	float rightThreat = ((200.f - cs.getOpponents(18)) / 200.f) + ((200.f - cs.getOpponents(19)) / 200.f) + ((200.f - cs.getOpponents(20)) / 200.f);

	float threat = leftThreat + rightThreat;


	//if (threat > 0.95f){
	//	_trackOffset = glm::mix(_trackOffset, -0.75f, 0.05f);
	//}
	//else{
	//	_trackOffset = glm::mix(_trackOffset, 0.75f, 0.05f);
	//}

	_trackOffset = 0.f;

	//if (_log)
	//	std::cout << threat << "\n";

	//std::cout << _trackOffset << "\n";

	//cc.getSteer

	//float steer = left - right;

	//_trackOffset = 0.f;


	float total = _trackOffset * (leftTrack + rightTrack);

	float steer = (leftTrack - total / 2) - (rightTrack + total / 2);



	




	//steer

	//float avoid = (left - right) - (cs.getOpponents(0) / 200.f);

	//std::cout << avoid << "\n";

	//cc.setSteer(changeRange(0, 360, -1, 1, _turningAngle));

	cc.setSteer(steer);
}

void MyDriver::onRestart(){

}

void MyDriver::onShutdown(){

}