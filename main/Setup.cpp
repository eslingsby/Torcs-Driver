#include <Setup.h>

#include <SimpleDriver.h>
#include "MainDriver.hpp"

typedef MainDriver Driver;

BaseDriver* setup(int argc, char *argv[]){
	Driver* driver = new Driver;

	return driver;
}