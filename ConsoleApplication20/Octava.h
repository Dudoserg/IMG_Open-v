#pragma once
#include "Pyramid_IMG.h"
#include "DoG_IMG.h"

#include <iostream>
#include <vector>



using namespace std;

class Octava
{
public:
	vector<Pyramid_IMG*> *images;
	vector<DoG_IMG*>* dog = new vector<DoG_IMG*>;

	Octava() {
		this->images = new vector<Pyramid_IMG*>();
	}

	Octava(vector<Pyramid_IMG*> *images) {
		this->images = images;
	}
};

