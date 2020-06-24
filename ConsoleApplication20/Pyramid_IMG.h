#pragma once
#include "IMG.h"
#include "Lab6_Descriptors_turn.h"
#include "Lab6_Harris.h"

class Pyramid_IMG
{
public:

	IMG *img = NULL;

	// ��� ����� ������� �����������
	Lab6_Harris* imgHarrisPart = NULL;

	// ��� ����������� �����������
	Lab6_Descriptors_turn* descriptors;

	double sigma;
	double globalSigma;
	int layerNum;
	int octavaNum;

	Pyramid_IMG() {

	}
};
