#include "MyDriver.hpp"

MyDriver::MyDriver(){

}

MyDriver::~MyDriver(){
	
}

void MyDriver::init(float* angles){
	// -90, -75, -60, -45, -30, 20, 15, 10, 5, 0, 5, 10, 15, 20, 30, 45, 60, 75, 90

	for (int i = 0; i < 5; i++){
		angles[i] = -90 + i * 15;
		angles[18 - i] = 90 - i * 15;
	}

	angles[9] = 0;

	for (int i = 5; i < 9; i++){
		angles[i] = -20 + (i - 5) * 5;
		angles[18 - i] = 20 - (i - 5) * 5;
	}
}

CarControl MyDriver::wDrive(CarState cs){
	_timePrevious = _timeCurrent;
	_timeCurrent = Clock::now();

	_dt = _timeCurrent - _timePrevious;
	_runtime += _dt;

	std::cout << _dt.count() << "\n";
	
	CarControl cc;

	steer(cs, cc);

	float forward = cs.getTrack((TRACK_SENSORS_NUM - 1) / 2) / _forwardDivisor;

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
	float left = cs.getTrack(0 + 2);
	float right = cs.getTrack(TRACK_SENSORS_NUM - 1 - 2);

	cc.setSteer(left - right);
}

void MyDriver::onRestart(){

}

void MyDriver::onShutdown(){

}