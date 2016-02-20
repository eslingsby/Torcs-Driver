#include "TorcsClient.hpp"

#include "SimpleParser.h"
#include <chrono>
#include <thread>

void race_thread(const std::string& hostname, unsigned int port, BaseDriver* driver){
	int code = TorcsClient::RESET;

	while (code == TorcsClient::RESET){
		TorcsClient client;

		if (!client.init(hostname, port))
			return;

		client.setDriver(driver);

		if (!client.connect(0)) // 0 = will never stop requesting
			break;

		code = client.run();
	}

	if (code == TorcsClient::ABORT)
		std::cout << "Something went wrong!\n";
}

bool TorcsClient::init(const std::string& hostname, unsigned int port){
	_hostName = hostname;
	_serverPort = port;

	// WSA Startup
	WSADATA wsaData = { 0 };
	WORD wVer = MAKEWORD(2, 2);
	int nRet = WSAStartup(wVer, &wsaData);

	if (nRet == SOCKET_ERROR){
		std::cout << "Failed to init WinSock library!\n";
		return false;
	}

	// Resolve hostname
	hostent* hostInfo = nullptr;

	hostInfo = gethostbyname(_hostName.c_str());

	if (hostInfo == nullptr){
		std::cout << "Problem interpreting host '" << _hostName.c_str() << "'!\n";
		return false;
	}

	// Create socket
	_socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (INVALID(_socketDescriptor)){
		std::cout << "Cannot create socket!\n";
		return false;
	}

	// Bind socket
	_serverAddress.sin_family = hostInfo->h_addrtype;

	memcpy((char*)&_serverAddress.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
	_serverAddress.sin_port = htons(_serverPort);

	return true;
}

bool TorcsClient::connect(unsigned int attempts){
	fd_set readSet;
	timeval timeVal;

	float angles[19];
	_driver->init(angles);

	std::string initString = "SCR" + SimpleParser::stringify("init", angles, 19);

	unsigned int attempt = 0;
	bool silent = false;

	while (!_connected){
		attempt += 1;

		if (sendto(_socketDescriptor, initString.c_str(), initString.length() * sizeof(char), 0, (sockaddr*)&_serverAddress, sizeof(_serverAddress)) < 0){
			std::cout << "Cannot send data!\n";
			CLOSE(_socketDescriptor);
			return false;
		}

		// wait until answer comes back, for up to UDP_CLIENT_TIMEUOT micro sec

		FD_ZERO(&readSet);
		FD_SET(_socketDescriptor, &readSet);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

		if (select(_socketDescriptor + 1, &readSet, NULL, NULL, &timeVal)){
			// Read data sent by the solorace server

			memset(_buffer, 0x0, UDP_MSGLEN);  // Zero out the buffer.

			int numRead = recv(_socketDescriptor, _buffer, UDP_MSGLEN, 0);

			if (numRead < 0)
				std::cout << "Attempting connection...\n";
			else{
				std::cout << "Received: " << _buffer << "\n";

				if (strcmp(_buffer, "***identified***") == 0)
					_connected = true;
			}
		}
		else if (!silent){
			std::cout << "No response, going silent...\n";
			silent = true;
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
	_driver = driver;
}

int TorcsClient::run(){
	if (_driver == nullptr)
		return 1;

	fd_set readSet;
	timeval timeVal;

	unsigned int maxTimeout = 16;
	unsigned int curTimeout = maxTimeout;

	bool reset = false;

	while (_connected){
		FD_ZERO(&readSet);
		FD_SET(_socketDescriptor, &readSet);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = UDP_CLIENT_TIMEUOT;

		if (select(_socketDescriptor + 1, &readSet, NULL, NULL, &timeVal)){
			if (curTimeout != maxTimeout)
				curTimeout = maxTimeout;

			// Read data sent by the solorace server
			memset(_buffer, 0x0, UDP_MSGLEN);  // Zero out the buffer.

			int numRead = recv(_socketDescriptor, _buffer, UDP_MSGLEN, 0);

			if (numRead < 0){
				std::cout << "No response from server!";
				CLOSE(_socketDescriptor);
				return 1;
			}

			// Check for server messages
			if (strcmp(_buffer, "***shutdown***") == 0){
				_driver->onShutdown();
				std::cout << "Client shutdown!\n";

				reset = true;
				break;
			}

			if (strcmp(_buffer, "***restart***") == 0){
				_driver->onRestart();
				std::cout << "Client restart!\n";

				reset = true;
				break;
			}

			// Give state to driver
			std::string action = _driver->drive(std::string(_buffer));
			memset(_buffer, 0x0, UDP_MSGLEN);
			sprintf_s(_buffer, "%s", action.c_str());

			// Send return from driver to server
			if (sendto(_socketDescriptor, _buffer, strlen(_buffer) + 1, 0, (sockaddr*)&_serverAddress, sizeof(_serverAddress)) < 0){
				std::cout << "Cannot send data!\n";
				CLOSE(_socketDescriptor);
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

	CLOSE(_socketDescriptor);

	WSACleanup();

	if (reset)
		return -1;

	return 0;
}