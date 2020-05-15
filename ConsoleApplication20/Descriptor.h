#pragma once


#include <iostream>
#include <vector>

using namespace std;


class Descriptor
{
public:
	int x;
	int y;

	vector<double> *vector;

	Descriptor(int x, int y, std::vector<double>* vector) {
		this->x = x;
		this->y = y;
		this->vector = vector;
	}
};

