#include <thread>
#include <list>
#include <string>

#include "Renderer.hpp"
#include "MyDriver.hpp"

#include <SimpleDriver.h>
#include <TorcsClient.hpp>

#define RENDERING 1

int main(int argc, char* argv[]){
	std::string hostname = "localhost";
	unsigned int firstPort = 3001;

	std::list<BaseDriver*> drivers;

	drivers.push_back(new MyDriver(true));
	//drivers.push_back(new SimpleDriver);
	//drivers.push_back(new SimpleDriver);
	//drivers.push_back(new SimpleDriver);
	
	std::list<std::thread> threads;

	for (BaseDriver* driver : drivers){
		threads.push_back(std::thread(race_thread, "localhost", firstPort + threads.size(), driver));
	}

	if (RENDERING){
		Renderer::get().setWindowSize({ 640, 640 });
		Renderer::get().init();

		while (Renderer::get().running()){
			Renderer::get().update();

			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	}

	for (std::thread& thread : threads){
		thread.join();
	}

	return 0;
}