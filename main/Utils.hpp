#pragma once


float changeRange(float oldMin, float oldMax, float newMin, float newMax, float oldValue);

float randomRange(int range, int spread);

float clamp(float min, float max, float value);

float lerp(float v0, float v1, float t);

float nearestEven(float n);