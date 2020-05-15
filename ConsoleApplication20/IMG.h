#pragma once
#define _USE_MATH_DEFINES

#include "DEF.h"
#include "Pixel.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include<opencv2/opencv.hpp>
#include <windows.h>
#include <string>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

class IMG
{
private:

	//double  *imageDoubleArr;
	//Pixel **pixels;
	bool IS_DEBUG = true;
	vector<Pixel*> *list;






public:
	Mat image;
	int width;
	int height;
	enum COLOR { RED, GREEN, BLUE, GRAY };

	IMG(string str) {
		this->list = new vector<Pixel*>();

		Mat img_tmp = imread(str);


		this->height = img_tmp.rows;
		this->width = img_tmp.cols;

		//QImage *imgOrig = new QImage(QString::fromStdString(str));
		this->setImage(img_tmp);
		this->initImageDoubleArr();
	}

	IMG(int w, int h, vector<Pixel*> *list) {
		this->width = w;
		this->height = h;
		this->list = list;
	}

	IMG() {

	}

	~IMG() {
#ifdef CLEAR_MEMORY
		this->deletePixels();
		this->image.release();
#endif // DEBUG_MODE
	}

	enum edgeEffect { BLACK, COPY, MIRROR, SPHERE };

	// Инициализация картинки надо выполнять сразу после  setImage
	void initImageDoubleArr();

	int getSize() {
		return this->width * this->height;
	}

	// Переводит картинку в оттенки серого
	//void createGrayScale( );

	IMG * downSample();

	
	void deletePixels();


	// Создать картинку из массива пикселей
	Mat createImage_GRAY();
	Mat createImage_COLOR();


	// Сохраняет картирку
	void saveQimageToFile(Mat img, string fileName, string dir);
	void saveImage_GRAY(string fileName, string dir);
	void saveImage_COLOR(string fileName, string dir);



	// Применить фильтр к изображению
	IMG *cross_COLOR(vector<vector<double>> &matrix, double div, edgeEffect e);
	IMG *cross_GRAY(vector<vector<double>> &matrix, double div, edgeEffect e);






	// нормализуем массив пикселей, приводим значение к промежутку [0...1]
	IMG  * normalize_COLOR();
	IMG  * normalize_GRAY();


	IMG * sobelDerivativeX(edgeEffect e);
	IMG * sobelDerivativeY(edgeEffect e);

	IMG * sobelGradient(IMG *pixelX, IMG *pixelY);

	IMG * gaussFilter(double sigma);

	IMG * gaussFilter_separable(double sigma);


	Mat getImage() const;
	void setImage(Mat value);
	vector<Pixel*>  *getPixels() const;
	void setPixels(vector<Pixel*> *value);


	double getColor(int coordinate, IMG::COLOR color);



	double colorPoint_RED = 0.9999999999999;
	double colorPoint_GREEN = 0.9999999999999;
	double colorPoint_BLUE = 0.9999999999999;
	void setRedSquare(int row, int col, int POINT_SIZE) {

		Pixel* basePixel = this->list->at(row * this->width + col);

		basePixel->red = colorPoint_RED;
		basePixel->green = colorPoint_GREEN;
		basePixel->blue = colorPoint_BLUE;
		basePixel->gray = 0.0;

		// красим пиксели вокруг в красный цвет
		int tmp_row_start = max(row - POINT_SIZE / 2, 0);
		int tmp_col_start = max(col - POINT_SIZE / 2, 0);
		int tmp_row_finish = min(row + POINT_SIZE / 2, this->height - 1);
		int tmp_col_finish = min(col + POINT_SIZE / 2, this->width - 1);

		for (int y = tmp_row_start; y <= tmp_row_finish; y++) {
			for (int x = tmp_col_start; x <= tmp_col_finish; x++) {
				Pixel* pixel_around = this->list->at(y * this->width + x);

				pixel_around->red = colorPoint_RED;
				pixel_around->green = colorPoint_GREEN;
				pixel_around->blue = colorPoint_BLUE;
				pixel_around->gray = 0.0;
			}
		}
	}



	vector<vector<double>*>* getGaussMatrix(int size) {
		vector<Pixel*>*  pixel = this->list;



		vector<vector<double>*>* matrix_gauss = new vector<vector<double>*>;

		//        int size = (int) (3 * sigma);
		double sigma = (double)(size / 3.0);
		int halfSize = size / 2;
		double ss2 = 2 * sigma * sigma;
		double firstDrob = 1.0 / (M_PI * ss2);
		double test_sum = 0.0;
		for (int x = -halfSize; x <= halfSize; x++) {
			vector<double>* tmp = new vector<double>();
			for (int y = -halfSize; y <= halfSize; y++) {
				double gauss = firstDrob * exp(-(x * x + y * y) / ss2);
				tmp->push_back(gauss);
				test_sum += gauss;
			}
			matrix_gauss->push_back(tmp);
		}
		double test_sum_after = 0.0;
		for (int x = -halfSize; x <= halfSize; x++) {
			for (int y = -halfSize; y <= halfSize; y++) {
				double val = matrix_gauss->at(x + halfSize)->at(y + halfSize) / test_sum;
				matrix_gauss->at(x + halfSize)->at(y + halfSize) = val;

				test_sum_after += matrix_gauss->at(x + halfSize)->at(y + halfSize);
			}
		}
		return matrix_gauss;
	}

	Pixel* getPixelWithEdge(int index_Y, int index_X) {
		index_X = index_X < 0 ? 0 : min(index_X, this->width - 1);
		index_Y = index_Y < 0 ? 0 : min(index_Y, this->height - 1);
		return this->list->at(index_Y * this->width + index_X);
	}


	IMG* copy() {
		IMG* new_object = new IMG();
		int size = this->width * this->height;

		new_object->list = new vector<Pixel*>;

		for (int i = 0; i < size; ++i)
			new_object->list->push_back(NULL);

		new_object->width = this->width;
		new_object->height = this->height;


		for (int i = 0; i < size; ++i)
			new_object->list->at(i) = this->list->at(i)->copy();

		return new_object;
	}



	static IMG* atan(IMG* imgX, IMG* imgY, int width, int height)
	{

		vector<Pixel*>* pixelX = imgX->list;
		vector<Pixel*>* pixelY = imgY->list;
		vector<Pixel*>* result_XY = new vector<Pixel*>();

		for (int i = 0; i < width * height; i++)
			result_XY->push_back(NULL);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int index = i * width + j;
				double val_X = (*pixelX)[index]->gray;
				double val_Y = (*pixelY)[index]->gray;
				//double sq = sqrt(val_X * val_X + val_Y * val_Y);
				double sq = atan2(val_X, val_Y);

				(*result_XY)[index] = new Pixel(sq);
			}
		}

		return new IMG(width, height, result_XY);
	}


	static IMG* sobolGradient(IMG* imgX, IMG* imgY, int width, int height)
	{

		vector<Pixel*>* pixelX = imgX->list;
		vector<Pixel*>* pixelY = imgY->list;
		vector<Pixel*>* result_XY = new vector<Pixel*>();

		for (int i = 0; i < width * height; i++)
			result_XY->push_back(NULL);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int index = i * width + j;
				double val_X = (*pixelX)[index]->gray;
				double val_Y = (*pixelY)[index]->gray;
				double sq = sqrt(val_X * val_X + val_Y * val_Y);
				//sq = qMax(qMin(sq, 255.0), 0.0);
				(*result_XY)[index] = new Pixel(sq);
			}
		}

		return new IMG(width, height, result_XY);
	}



	// Поме
	void setMatToPixelsArray(Mat mat) {


		for (int row = 0; row < this->height; ++row) {
			for (int col = 0; col < this->width; ++col) {
				Vec3b& color = (mat.at<Vec3b>(row, col));
				int blue = color[0] ;
				int green = color[1];
				int red = color[2];
				int gray = ((double)red * 0.213) + ((double)green * 0.715) + ((double)blue * 0.0722);

				list->at(row * this->width + col)->red = red / 255.0;
				list->at(row * this->width + col)->green = green / 255.0;
				list->at(row * this->width + col)->blue = blue / 255.0;
				list->at(row * this->width + col)->gray = gray / 255.0;
			}
		}

	}
};