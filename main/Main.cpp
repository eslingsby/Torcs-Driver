#include <thread>
#include "Renderer.hpp"

#include "MyDriver.hpp"
#include <SimpleDriver.h>
#include <TorcsClient.hpp>

int main(int argc, char* argv[]){
	std::thread thread_0(race_thread, "localhost", 3001, new MyDriver(true));
	//std::thread thread_1(race_thread, "localhost", 3002, new MyDriver(true));
	//std::thread thread_2(race_thread, "localhost", 3003, new MyDriver);
	
	Renderer::get().init();

	while (Renderer::get().running()){
		Renderer::get().update();

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	thread_0.join();
	//thread_1.join();
	//thread_2.join();

	return 0;
}