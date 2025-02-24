#pragma once
#include <random>

class RNG
{
public:
	static inline float GetRandomFloatNumber(float minValue, float maxValue)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(minValue, maxValue);
		return dist(gen);
	}
};
