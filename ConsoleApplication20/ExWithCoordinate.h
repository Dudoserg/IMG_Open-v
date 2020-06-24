#pragma once
class ExWithCoordinate
{
public:
    int row;
    int col;
    double extremum;

    ExWithCoordinate(int row, int col, double extremum) {
        this->row = row;
        this->col = col;
        this->extremum = extremum;
    }
};

