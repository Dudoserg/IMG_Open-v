#pragma once
#include "IMG.h"

class Pyramid_IMG
{
public:

	IMG *img = NULL;
	double sigma;
	double globalSigma;
	int layerNum;
	int octavaNum;

	Pyramid_IMG() {

	}
};
