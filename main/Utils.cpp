#include "Utils.hpp"

float changeRange(float oldMin, float oldMax, float newMin, float newMax, float oldValue){
	return (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
}

float lerp(float v0, float v1, float t){
	return (1 - t)*v0 + t*v1;
}