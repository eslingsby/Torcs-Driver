#include <thread>
#include "Renderer.hpp"

#include "MyDriver.hpp"
#include <SimpleDriver.h>
#include <TorcsClient.hpp>

int main(int argc, char* argv[]){
	BaseDriver* driver = new SimpleDriver;

	std::thread thread(race_thread, "localhost", 3001, driver);

	//Renderer::get().init();
	//
	//while (Renderer::get().running()){ //&& driver is racing
	//	Renderer::get().update();
	//}

	thread.join();

	return 0;
}