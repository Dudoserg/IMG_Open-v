#pragma once
#define _USE_MATH_DEFINES

#include "DEF.h"
//#include "Pixel.h"

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
	//vector<Pixel*> *list;


	



public:
	Mat image;
	int width;
	int height;


	double* pixels_red;
	double* pixels_green;
	double* pixels_blue;
	double* pixels_gray;

	enum COLOR { RED, GREEN, BLUE, GRAY };

	IMG(string str) {
		//this->list = new vector<Pixel*>();

		Mat img_tmp = imread(str);
		


		this->height = img_tmp.rows;
		this->width = img_tmp.cols;

		this->pixels_red = new double[this->height * this->width];
		this->pixels_green = new double[this->height * this->width];
		this->pixels_blue = new double[this->height * this->width];
		this->pixels_gray = new double[this->height * this->width];

		//QImage *imgOrig = new QImage(QString::fromStdString(str));
		this->setImage(img_tmp);
		this->initImageDoubleArr();
	}

	IMG(int w, int h, double* r, double* g, double* b, double* gray) {
		this->width = w;
		this->height = h;
		this->pixels_red = r;
		this->pixels_green = g;
		this->pixels_blue = b;
		this->pixels_gray = gray;
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
	//vector<Pixel*>  *getPixels() const;
	//void setPixels(vector<Pixel*> *value);


	double getColor(int coordinate, IMG::COLOR color);



	double colorPoint_RED = 0.9999999999999;
	double colorPoint_GREEN = 0.9999999999999;
	double colorPoint_BLUE = 0.9999999999999;


	void setRedSquare(int row, int col, int POINT_SIZE) {

		this->pixels_red[row * this->width + col] = colorPoint_RED;
		this->pixels_green[row * this->width + col] = colorPoint_GREEN;
		this->pixels_blue[row * this->width + col] = colorPoint_BLUE;
		this->pixels_gray[row * this->width + col] = 0.0;

		// красим пиксели вокруг в красный цвет
		int tmp_row_start = max(row - POINT_SIZE / 2, 0);
		int tmp_col_start = max(col - POINT_SIZE / 2, 0);
		int tmp_row_finish = min(row + POINT_SIZE / 2, this->height - 1);
		int tmp_col_finish = min(col + POINT_SIZE / 2, this->width - 1);

		for (int y = tmp_row_start; y <= tmp_row_finish; y++) {
			for (int x = tmp_col_start; x <= tmp_col_finish; x++) {
				this->pixels_red[y * this->width + x] = colorPoint_RED;
				this->pixels_green[y * this->width + x] = colorPoint_GREEN;
				this->pixels_blue[y * this->width + x] = colorPoint_BLUE;
				this->pixels_gray[y * this->width + x] = 0.0;
			}
		}
	}


	static vector<vector<double>*>* static_getGaussMatrix(int size) {
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

	vector<vector<double>*>* getGaussMatrix(int size) {
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

	double getRedWithEdge(int index_Y, int index_X) {
		index_X = index_X < 0 ? 0 : min(index_X, this->width - 1);
		index_Y = index_Y < 0 ? 0 : min(index_Y, this->height - 1);
		return this->pixels_red[index_Y * this->width + index_X];
	}
	double getGreenWithEdge(int index_Y, int index_X) {
		index_X = index_X < 0 ? 0 : min(index_X, this->width - 1);
		index_Y = index_Y < 0 ? 0 : min(index_Y, this->height - 1);
		return this->pixels_green[index_Y * this->width + index_X];
	}
	double getBlueWithEdge(int index_Y, int index_X) {
		index_X = index_X < 0 ? 0 : min(index_X, this->width - 1);
		index_Y = index_Y < 0 ? 0 : min(index_Y, this->height - 1);
		return this->pixels_blue[index_Y * this->width + index_X];
	}
	double getGrayWithEdge(int index_Y, int index_X) {
		index_X = index_X < 0 ? 0 : min(index_X, this->width - 1);
		index_Y = index_Y < 0 ? 0 : min(index_Y, this->height - 1);
		return this->pixels_gray[index_Y * this->width + index_X];
	}



	IMG* copy() {
		IMG* new_object = new IMG();
		int size = this->width * this->height;


		new_object->width = this->width;
		new_object->height = this->height;


		double* result_red = new double[new_object->width * new_object->height];
		double* result_green = new double[new_object->width * new_object->height];
		double* result_blue = new double[new_object->width * new_object->height];
		double* result_gray = new double[new_object->width * new_object->height];

		new_object->pixels_red = result_red;
		new_object->pixels_green = result_green;
		new_object->pixels_blue = result_blue;
		new_object->pixels_gray = result_gray;

		for (int i = 0; i < size; ++i) {
			result_red[i] = this->pixels_red[i];
			result_green[i] = this->pixels_green[i];
			result_blue[i] = this->pixels_blue[i];
			result_gray[i] = this->pixels_gray[i];
		}

		return new_object;
	}



	static IMG* atan(IMG* imgX, IMG* imgY, int width, int height)
	{
		double* result_red = new double[width * height];
		double* result_green = new double[width * height];
		double* result_blue = new double[width * height];
		double* result_gray = new double[width * height];

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int index = i * width + j;
				double val_X = imgX->pixels_gray[index];
				double val_Y = imgY->pixels_gray[index];
			
				double sq = atan2(val_Y, val_X);

				result_gray[index] = sq;
			}
		}

		return new IMG(width, height, result_red, result_green, result_blue, result_gray);
	}


	static IMG* sobolGradient(IMG* imgX, IMG* imgY, int width, int height)
	{
		double* result_red = new double[width * height];
		double* result_green = new double[width * height];
		double* result_blue = new double[width * height];
		double* result_gray = new double[width * height];

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				int index = i * width + j;
				double val_X = imgX->pixels_gray[index];
				double val_Y = imgY->pixels_gray[index];
				double sq = sqrt(val_X * val_X + val_Y * val_Y);
				//sq = qMax(qMin(sq, 255.0), 0.0);
				result_gray[index] = sq;
			}
		}

		return new IMG(width, height, result_red, result_green, result_blue, result_gray);
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

				this->pixels_red[row * this->width + col] = red / 255.0;
				this->pixels_green[row * this->width + col] = green / 255.0;
				this->pixels_blue[row * this->width + col] = blue / 255.0;
				this->pixels_gray[row * this->width + col] = gray / 255.0;
			}
		}
	}
};