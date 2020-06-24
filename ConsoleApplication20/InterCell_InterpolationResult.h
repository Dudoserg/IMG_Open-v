#pragma once

#include <string>
#include <iostream>
using namespace std;


class InterCell_InterpolationResult
{
public:
    // ���������� �����������, ���� �������� ��������
     int index_y;
     int index_x;
    // �������� � ������ �� 1 , ��������
     double  fraction;
    // diagonal vertical horisontal
     string str;


     InterCell_InterpolationResult(int index_y, int index_x, double fraction, string str) {
        this->index_y = index_y;
        this->index_x = index_x;
        this->fraction = fraction;
        this->str = str;
    }
};

