#pragma once

#include "Descriptor.h"

class InterestingPoint
{
public:
    int row;
    int col;

    // в радианах
    double angle_rad;

    // в коробках
    int angle_boxNum;

    double angle_gradient_TEST;

    Descriptor* descriptor;

    double getAngle_gradient_TEST() {
        return angle_gradient_TEST;
    }

    void setAngle_gradient_TEST(double angle_gradient_TEST) {
        this->angle_gradient_TEST = angle_gradient_TEST;
    }

    InterestingPoint(int row, int col) {
        this->row = row;
        this->col = col;
    }
};


