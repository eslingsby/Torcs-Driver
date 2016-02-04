#include "MyDriver.hpp"

MyDriver::MyDriver(){}

MyDriver::~MyDriver(){}

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
	_previousState = _currentState;
	_currentState = cs;

	_previousControl = _currentControl;
	_currentControl = CarControl();

	_previousTime = _currentTime;
	_currentTime = Clock::now();
	
	Clock::time_point dt(_currentTime - _previousTime);

	cout << Clock::to_time_t(dt) << endl;

	// Do stuff

	return _currentControl;
}

void MyDriver::onRestart(){
	cout << "Restarting race..." << endl;
}

void MyDriver::onShutdown(){
	cout << "Shutting down..." << endl;
}