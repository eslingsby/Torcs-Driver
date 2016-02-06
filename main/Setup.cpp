#include <Setup.h>

#include <SimpleDriver.h>
#include "MyDriver.hpp"

BaseDriver* setup(int argc, char *argv[]){
	return new MyDriver;
}