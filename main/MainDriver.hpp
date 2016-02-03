#pragma once

#include <WrapperBaseDriver.h>

class MainDriver : public WrapperBaseDriver{
	CarControl _lastControl;

public:
	MainDriver();

	~MainDriver();

	void init(float* angles);

	CarControl wDrive(CarState cs);

	void onRestart();
	void onShutdown();
};