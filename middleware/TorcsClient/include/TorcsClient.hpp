#pragma once

#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <BaseDriver.h>

#define UDP_MSGLEN 1000
#define UDP_CLIENT_TIMEUOT 1000000

#define CLOSE(x) closesocket(x)

#define INVALID(x) x == INVALID_SOCKET

void race_thread(const std::string& hostname, unsigned int port, BaseDriver* driver);

class TorcsClient{
	// Host information
	std::string hostName;
	unsigned int serverPort;

	SOCKET socketDescriptor;
	sockaddr_in serverAddress;

	char buffer[UDP_MSGLEN];

	BaseDriver* driver = nullptr;

	bool connected = false;

public:

	bool init(const std::string& hostname = "localhost", unsigned int port = 3001);

	bool connect(unsigned int attempts = 0);

	void setDriver(BaseDriver* driver);

	int run();
};