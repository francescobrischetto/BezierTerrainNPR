/*
Random Float  Util Class and Methods
*/
#pragma once
#include <random>
#include <ctime>

class RNG_float {
public:
	RNG_float() { rng.seed(std::random_device{}()); }
	float operator()(float min, float max) { 
	std::uniform_real_distribution<float> dist(min, max); return dist(rng); }
private:
	std::mt19937 rng;
};