#pragma once

#include "BaseDriver.h"

BaseDriver* setup(int argc, char *argv[]);

#ifdef _DEBUG
#pragma comment(lib, "../bin/TorcsClientd.lib")
#else
#pragma comment(lib, "../bin/TorcsClient.lib")
#endif