#include "Utils.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <random>
#include <glm\integer.hpp>

float changeRange(float oldMin, float oldMax, float newMin, float newMax, float oldValue){
	return (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
}

float randomRange(int range, int spread){
	return (float)((rand() % range * spread) - (range / 2) * spread);
}

float clamp(float min, float max, float value){
	if (min > value)
		return min;

	if (value > max)
		return max;

	return value;
}

float lerp(float v0, float v1, float t){
	return (1 - t)*v0 + t*v1;
}

float nearestEven(float n){
	int i = (int)glm::round(n);

	if (i % 2 == 0)
		return (float)n;

	return (float)(n + 1);
}