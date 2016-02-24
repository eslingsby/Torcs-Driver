#include <thread>
#include <list>
#include <string>

#include "Renderer.hpp"
#include "MyDriver.hpp"

#include <SimpleDriver.h>
#include <TorcsClient.hpp>

typedef std::list<BaseDriver*> DriverList;

void parseArgs(int argc, char* argv[], std::string& hostName, unsigned int& startPort, bool& rendering){
	if (argc < 2){
		hostName = "localhost";
		startPort = 3001;
	}
	else{
		hostName = argv[1];
		startPort = (unsigned)std::stoi(argv[2]);
	}
}

void makeDrivers(DriverList& drivers){
	drivers.push_back(new MyDriver(true));
	drivers.push_back(new MyDriver);
	drivers.push_back(new MyDriver);
	drivers.push_back(new MyDriver);
	//drivers.push_back(new SimpleDriver);
	//drivers.push_back(new SimpleDriver);
	//drivers.push_back(new SimpleDriver);
}

int main(int argc, char* argv[]){
	srand((time_t)time(0));

	// Set hostName and startPort using args, or default "localhost" and 3001
	std::string hostName;
	unsigned int startPort;

	bool rendering = true;

	if (argc == 2){
		std::cout << "Usage : 'Torcs-Driver.exe hostName startPort'\nExample : 'Torcs-Driver.exe 127.0.0.1 3001'\n";
		return 1;
	}
	
	parseArgs(argc, argv, hostName, startPort, rendering);

	std::cout << "Using host-name " << hostName << " starting at port " << startPort << "\n";

	// Creation of new driver pointers derivded from BaseDriver
	DriverList drivers;
	makeDrivers(drivers);
	
	// Creates individual threads for each driver to connect to a race with an incremented port number
	std::list<std::thread> threads;

	for (BaseDriver* driver : drivers)
		threads.push_back(std::thread(race_thread, hostName, startPort + threads.size(), driver));

	// Now the race has potentially started, initiate the debug renderer if rendering is on 
	if (rendering){
		Renderer::get().setWindowSize({ 640, 640 });
		Renderer::get().init();

		while (Renderer::get().running()){
			Renderer::get().update();

			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	}

	// Wait for each driver to finish
	for (std::thread& thread : threads)
		thread.join();

	// Kill everything
	for (BaseDriver* driver : drivers)
		delete driver;

	threads.clear();
	drivers.clear();

	return 0;
}