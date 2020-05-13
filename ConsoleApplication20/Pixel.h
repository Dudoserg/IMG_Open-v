#pragma once
class Pixel
{
public:
	double red;
	double green;
	double blue;
	double alpha;
	double gray;

	Pixel() {

	}
	Pixel(double gray) {
		this->gray = gray;
	}
	Pixel(double r, double g, double b, double a, double gray) {
		this->red = r;
		this->green = g;
		this->blue = b;
		this->alpha = a;
		this->gray = gray;
	}
	Pixel(double r, double g, double b, double a) {
		this->red = r;
		this->green = g;
		this->blue = b;
		this->alpha = a;

	}

	Pixel* copy() {
		Pixel* new_object = new Pixel();
		new_object->red = this->red;
		new_object->green = this->green;
		new_object->blue = this->blue;
		new_object->alpha = this->alpha;
		new_object->gray = this->gray;

		return new_object;
	}
};
