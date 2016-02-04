#include <Setup.h>

#include <SimpleDriver.h>
#include "MyDriver.hpp"

typedef MyDriver Driver;

BaseDriver* setup(int argc, char *argv[]){
	Driver* driver = new Driver;

	return driver;
}