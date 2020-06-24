#pragma once
#include "IMG.h"
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class HarrisPixel
{
public:

	int variant = 0;

	double A = INT_MIN;
	double B = INT_MIN;
	double C = INT_MIN;
	double L_min = INT_MIN;
	double L_max = INT_MIN;
	bool isActive = true;
	int row = INT_MIN;
	int col = INT_MIN;
	double _k = 0.05;

	double getL_min() {
		return L_min;
	}
	void setL_min(double l_min) {
		L_min = l_min;
	}

	HarrisPixel(int row, int col, double a, double b, double c) {
		A = a;
		B = b;
		C = c;
		this->row = row;
		this->col = col;

		// Вычисляем сразу собственное число
		pair<double, double>* ownNUmber = this->getOwnNUmber();
		this->L_min = ownNUmber->first;
		this->L_max = ownNUmber->second;
	}

	HarrisPixel() {
	}

	double sqr(double x) {
		return x * x;
	}

	pair<double, double>* getOwnNUmber() {
		switch (variant) {
		case 0: {
			double _b = A + C;
			double _c = A * C - B * B;
			double D = _b * _b - 4 * _c;
			double L_1 = (_b + sqrt(D)) / 2;
			double L_2 = (_b - sqrt(D)) / 2;
			double L_min = min(L_1, L_2);
			double L_max = max(L_1, L_2);
			return new pair<double, double>(L_min, L_max);
		}
		case 1: {
			//второй вариант, но он чет не оч
			double L_min_V2 = A * C - sqr(B) - _k * sqr(A + C);
			L_min_V2 = abs(L_min_V2);
			return new pair<double, double>(L_min_V2, 1238123.0);
		}
		default: {
			//второй вариант, но он чет не оч
			double L_min_V2 = A * C - sqr(B) - _k * sqr(A + C);
			L_min_V2 = abs(L_min_V2);
			return new pair<double, double>(L_min_V2, 1238123.0);
		}
		}
	}

	HarrisPixel* copy() {
		HarrisPixel *newPixel = new HarrisPixel();
		newPixel->A = this->A;
		newPixel->B = this->B;
		newPixel->C = this->C;
		newPixel->L_min = this->L_min;
		newPixel->L_max = this->L_max;
		newPixel->isActive = this->isActive;
		newPixel->row = this->row;
		newPixel->col = this->col;

		return newPixel;
	}
};