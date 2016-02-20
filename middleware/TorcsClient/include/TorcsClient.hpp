#pragma once

#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <BaseDriver.h>

#define UDP_MSGLEN 1000
#define UDP_CLIENT_TIMEUOT 1000000

#define ATTEMPT_SLEEP_TIME 1000

#define CLOSE(x) closesocket(x)

#define INVALID(x) x == INVALID_SOCKET

void race_thread(const std::string& hostname, unsigned int port, BaseDriver* driver);

class TorcsClient{
	std::string _hostName;
	unsigned int _serverPort;

	SOCKET _socketDescriptor;
	sockaddr_in _serverAddress;

	char _buffer[UDP_MSGLEN];

	BaseDriver* _driver = nullptr;

	bool _connected = false;

public:
	enum RUN_CODES{
		RESET = -1,
		SHUTDOWN = 0,
		ABORT = 1
	};

	bool init(const std::string& hostname, unsigned int port);

	bool connect(unsigned int attempts = 0);

	void setDriver(BaseDriver* driver);

	int run();
};