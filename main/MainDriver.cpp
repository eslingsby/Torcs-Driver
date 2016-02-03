#include "MainDriver.hpp"

MainDriver::MainDriver(){

}

MainDriver::~MainDriver(){

}

void MainDriver::init(float* angles){
	// -90, -75, -60, -45, -30, 20, 15, 10, 5, 0, 5, 10, 15, 20, 30, 45, 60, 75, 90

	for (int i = 0; i<5; i++){
		angles[i] = -90 + i * 15;
		angles[18 - i] = 90 - i * 15;
	}

	angles[9] = 0;

	for (int i = 5; i<9; i++){
		angles[i] = -20 + (i - 5) * 5;
		angles[18 - i] = 20 - (i - 5) * 5;
	}
}

CarControl MainDriver::wDrive(CarState cs){
	CarControl control;

	// stuff here

	_lastControl = control;

	return _lastControl;
}

void MainDriver::onRestart(){
	cout << "Restarting the race!" << endl;
}

void MainDriver::onShutdown(){
	cout << "Bye bye!" << endl;
}