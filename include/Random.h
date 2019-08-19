#pragma once
#include <random>
#include <chrono>
#include <iostream>
#include <Windows.h>

extern std::default_random_engine g_randomGenerator;

template<class T>
class RandomNormal {
private:
	T _mean;
	T _sigma;
	T _min;
	T _max;
	std::normal_distribution<T> s_dist;
public:
	RandomNormal(T mean, T sigma, T min, T max) :
		_mean(mean),
		_sigma(sigma),
		_min(min),
		_max(max),
		generator(42),
		s_dist(_mean, _sigma) {
	}
	T generate() {

		float rand_num = _min - 1;

		while (rand_num < _min || rand_num > _max) rand_num = s_dist(g_randomGenerator);

		return rand_num;
	}

};

template<class T>
class UniformRandom {
private:
	T _min;
	T _max;
	std::uniform_int_distribution<int> _distributionInt;
	std::uniform_real_distribution<float> _distributionFloat;

public:
	void setParams(T min, T max)
	{
		_min = min; _max = max;
		if (std::is_same<T, int>::value) {
			_distributionInt = std::uniform_int_distribution<int>(_min, _max);
		}
		else {
			_distributionFloat = std::uniform_real_distribution<float>(_min, _max);
		}
	}

	T generate()
	{
		if (std::is_same<T, int>::value) {
			return _distributionInt(g_randomGenerator);
		}
		else {
			return _distributionFloat(g_randomGenerator);
		}
	}
};