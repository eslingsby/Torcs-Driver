#include "TorcsClient.hpp"

#include "SimpleParser.h"
#include <chrono>
#include <thread>

void race_thread(const std::string& hostname, unsigned int port, BaseDriver* driver){
	int code = -1;

	while (code == -1){
		TorcsClient client;

		if (!client.init(hostname, port))
			return;

		client.setDriver(driver);

		if (!client.connect(0)) // 0 = will never stop requesting
			break;

		code = client.run();
	}

	if (code == 1)
		std::cout << "Something went wrong!\n";
}

bool TorcsClient::init(const std::string& hostname, unsigned int port){
	this->hostName = hostname;
	this->serverPort = port;

	// WSA Startup
	WSADATA wsaData = { 0 };
	WORD wVer = MAKEWORD(2, 2);
	int nRet = WSAStartup(wVer, &wsaData);

	if (nRet == SOCKET_ERROR){
		std::cout << "Failed to init WinSock library" << std::endl;
		return false;
	}

	// Resolve hostname
	hostent* hostInfo = NULL;

	hostInfo = gethostbyname(hostName.c_str());

	if (hostInfo == NULL){
		std::cout << "Error: problem interpreting host: " << hostName.c_str() << "\n";
		return false;
	}

	// Create socket
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (INVALID(socketDescriptor)){
		std::cout << "Cannot create socket!\n";
		return false;
	}

	// Bind socket
	serverAddress.sin_family = hostInfo->h_addrtype;

	memcpy((char*)&serverAddress.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
	serverAddress.sin_port = htons(serverPort);

	return true;
}

bool TorcsClient::connect(unsigned int attempts){
	fd_set readSet;
	timeval timeVal;

	float angles[19];
	driver->init(angles);

	string initString = "SCR" + SimpleParser::stringify(string("init"), angles, 19);

	unsigned int attempt = 0;

	while (!connected){
		attempt += 1;

		if (sendto(socketDescriptor, initString.c_str(), initString.length() * sizeof(char), 0, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
			std::cout << "Cannot send data!\n";
			CLOSE(socketDescriptor);
			return false;
		}

		// wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec

		FD_ZERO(&readSet);
		FD_SET(socketDescriptor, &readSet);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

		if (select(socketDescriptor + 1, &readSet, NULL, NULL, &timeVal)){
			// Read data sent by the solorace server

			memset(buffer, 0x0, UDP_MSGLEN);  // Zero out the buffer.

			int numRead = recv(socketDescriptor, buffer, UDP_MSGLEN, 0);

			if (numRead < 0){
				std::cout << "Attempting connection!\n";
			}
			else{
				std::cout << "Received: " << buffer << "\n";

				if (strcmp(buffer, "***identified***") == 0)
					connected = true;
			}
		}

		if (attempts != 0 && attempt >= attempts){
			std::cout << "Failed after " << attempt << " attempts...\n";
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return true;
}

void TorcsClient::setDriver(BaseDriver* driver){
	this->driver = driver;
}

int TorcsClient::run(){
	if (driver == nullptr)
		return 1;

	fd_set readSet;
	timeval timeVal;

	unsigned int maxTimeout = 16;
	unsigned int curTimeout = maxTimeout;

	bool reset = false;

	while (connected){
		FD_ZERO(&readSet);
		FD_SET(socketDescriptor, &readSet);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

		if (select(socketDescriptor + 1, &readSet, NULL, NULL, &timeVal)){
			if (curTimeout != maxTimeout)
				curTimeout = maxTimeout;

			// Read data sent by the solorace server
			memset(buffer, 0x0, UDP_MSGLEN);  // Zero out the buffer.

			int numRead = recv(socketDescriptor, buffer, UDP_MSGLEN, 0);

			if (numRead < 0){
				std::cout << "No response from server!";
				CLOSE(socketDescriptor);
				return 1;
			}

			// Check for server messages
			if (strcmp(buffer, "***shutdown***") == 0){
				driver->onShutdown();
				std::cout << "Client shutdown!\n";

				reset = true;
				break;
			}

			if (strcmp(buffer, "***restart***") == 0){
				driver->onRestart();
				std::cout << "Client restart!\n";

				reset = true;
				break;
			}

			// Give state to driver
			std::string action = driver->drive(std::string(buffer));
			memset(buffer, 0x0, UDP_MSGLEN);
			sprintf_s(buffer, "%s", action.c_str());

			// Send return from driver to server
			if (sendto(socketDescriptor, buffer, strlen(buffer) + 1, 0, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
				std::cout << "Cannot send data!\n";
				CLOSE(socketDescriptor);
				return 1;
			}
		}
		else{
			curTimeout--;

			if (curTimeout == 0)
				return -1;

			std::cout << "Server did not respond, " << curTimeout << " retries left...\n";
		}
	}

	CLOSE(socketDescriptor);

	WSACleanup();

	if (reset)
		return -1;

	return 0;
}