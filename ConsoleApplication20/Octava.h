#pragma once
#include "Pyramid_IMG.h"

#include <iostream>
#include <vector>



using namespace std;

class Octava
{
public:
	vector<Pyramid_IMG*> *images;

	Octava() {
		this->images = new vector<Pyramid_IMG*>();
	}

	Octava(vector<Pyramid_IMG*> *images) {
		this->images = images;
	}
};

