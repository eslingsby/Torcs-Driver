#pragma once

#include "BaseDriver.h"

BaseDriver* setup(int argc, char *argv[]);

#ifdef _DEBUG
#pragma comment(lib, "../bin/SimpleClientd.lib")
#else
#pragma comment(lib, "../bin/SimpleClient.lib")
#endif