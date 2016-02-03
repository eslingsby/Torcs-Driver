#include "Setup.hpp"

#include <SimpleDriver.h>
#include <ANNdriver.h>

BaseDriver* setup(int argc, char *argv[]){
	return new SimpleDriver;
}