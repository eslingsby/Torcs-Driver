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
	
	CarControl cc;

	float slowdown = steer(cs, cc);

	cout << slowdown << endl;

	if (getSpeed(cs) < _maxSpeed - slowdown * 60.f){
		if (cs.getGear() == 0)
			engageGear(cc);
		else if (cs.getRpm() > 5000 && cs.getGear() != 6)
			engageGear(cc, cs.getGear() + 1);


		if (!_clutching)
			cc.setAccel(1.f);
	}
	else{
		cc.setAccel(0.f);
		//cc.setBrake(slowdown);
	}

	//cout << "Speed : " << getSpeed(cs) << " Delta Time : " << _dt.count() << endl;

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

float MyDriver::steer(CarState& cs, CarControl& cc){
	//float left = cs.getTrack(0 + 2);
	//float right = cs.getTrack(TRACK_SENSORS_NUM - 1 - 2);
	//float forward = cs.getTrack((TRACK_SENSORS_NUM - 1) / 2);

	//cout << "L-" << left << " R-" << right << " F-" << forward << endl;

	float position = cs.getTrackPos();

	float value = position;
	cc.setSteer(-position);

	return glm::abs(value);	
}

void MyDriver::onRestart(){

}

void MyDriver::onShutdown(){

}