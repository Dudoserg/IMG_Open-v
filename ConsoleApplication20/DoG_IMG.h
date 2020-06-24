#pragma once
#include "IMG.h"
#include "ExWithCoordinate.h"
class DoG_IMG
{
public:
	IMG* img = NULL;
	vector<ExWithCoordinate*>* extremumCoordinates = new vector<ExWithCoordinate*>;
};

