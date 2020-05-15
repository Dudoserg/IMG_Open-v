//

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include <vector>

#include "IMG.h"
#include "DEF.h"

using namespace std;
using namespace cv;

Mat IMG::getImage() const
{
	return image;

}

void IMG::setImage(Mat value)
{
	image = value;
	this->height = value.rows;
	this->width = value.cols;
}

vector<Pixel*>* IMG::getPixels() const
{
	return this->list;
}



void IMG::setPixels(vector<Pixel*>* value)
{
	int size = this->width * this->height;
	//    for(int i=0; i < size; ++i)
	//        delete this->pixels[i];
	//    delete [] this->pixels;

	list = value;
}





// Создаем одномерный массив **Pixel, в котором храним цвет (Приведя его к промежутку [0...1])
void IMG::initImageDoubleArr()
{
	Pixel** pixels = new Pixel * [this->width * this->height];

	for (int row = 0; row < this->height; ++row) {
		for (int col = 0; col < this->width; ++col) {
			Vec3b& color = (this->image.at<Vec3b>(row, col));
			int blue = color[0];
			int green = color[1];
			int red = color[2];
			//int gray = ((double)red * 0.2125) + ((double)green * 0.7154) + ((double)blue * 0.0721);
			int gray = ((double)red * 0.213) + ((double)green * 0.715) + ((double)blue * 0.0722);

			list->push_back(new Pixel((red / 255.0), (green / 255.0), (blue / 255.0), (gray / 255.0), gray / 255.0));
		}
	}
}






void IMG::deletePixels()
{
#ifdef CLEAR_MEMORY
	for (int i = 0; i < this->list->size(); ++i)
		delete (*list)[i];

	this->list->clear();
	this->list->shrink_to_fit();

	delete this->list;
#endif // CLEAR_MEMORY
}



void IMG::saveQimageToFile(Mat result, string fileName, string dir)
{
	fileName = (dir + "/" + fileName);


	imwrite(fileName, result);

}

void IMG::saveImage_GRAY(string fileName, string dir)
{

	Mat tmp_image = this->createImage_GRAY();
	this->saveQimageToFile(tmp_image, fileName, dir);
	//delete tmp_image;

}


void IMG::saveImage_COLOR(string fileName, string dir)
{
	unsigned int start_time = clock();
	Mat tmp_image = this->createImage_COLOR();
	this->saveQimageToFile(tmp_image, fileName, dir);
	//delete tmp_image;
	cout << "savePixeslToFile : " << (clock() - start_time) / 1000.0 << "\n\n\n";
}

/**
 * @brief IMG::cross
 * @param pixel исходный массив пикселей
 * @param matrix матрица фильтр
 * @param div   делитель для матрицы фильтр
 * @param e     краевой эффект
 * @return      новый массив пикселей
 */
IMG* IMG::cross_COLOR(vector<vector<double> >& matrix, double div, edgeEffect e)
{
	vector<Pixel*>* pixel = this->list;


	//    Pixel **result =  new Pixel*[this->width * this->height];

	vector<Pixel*>* result = new vector<Pixel*>();
	for (int i = 0; i < this->width * this->height; i++)
		result->push_back(new Pixel(
			(*pixel)[i]->red,
			(*pixel)[i]->green,
			(*pixel)[i]->blue,
			(*pixel)[i]->alpha,
			(*pixel)[i]->gray
		));



	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int ku = matrix.size() / 2;
			int kv = matrix[0].size() / 2;
			double sum_GRAY = 0.0;
			double value_GRAY = 0.0;

			double sum_RED = 0.0;
			double value_RED = 0.0;

			double sum_GREEN = 0.0;
			double value_GREEN = 0.0;

			double sum_BLUE = 0.0;
			double value_BLUE = 0.0;

			for (int u = -ku; u <= ku; u++) {
				for (int v = -kv; v <= kv; v++) {
					int x = row - u;
					int y = col - v;


					if (x < 0 || y < 0 || x >= this->height || y >= this->width) {

						if (x < 0)
							x = x + this->height;
						else if (x >= this->height)
							x = x - this->height;

						if (y < 0)
							y = y + this->width;
						else if (y >= this->width)
							y = y - this->width;



					}
					value_RED = (*pixel)[x * this->width + y]->red;
					value_GREEN = (*pixel)[x * this->width + y]->green;
					value_BLUE = (*pixel)[x * this->width + y]->blue;
					value_GRAY = (*pixel)[x * this->width + y]->gray;

					sum_RED += matrix[u + ku][v + kv] * value_RED;
					sum_GREEN += matrix[u + ku][v + kv] * value_GREEN;
					sum_BLUE += matrix[u + ku][v + kv] * value_BLUE;
					sum_GRAY += matrix[u + ku][v + kv] * value_GRAY;
				}
			}
			sum_RED *= div;
			sum_GREEN *= div;
			sum_BLUE *= div;
			sum_GRAY *= div;
			(*result)[row * this->width + col]->red = sum_RED;
			(*result)[row * this->width + col]->green = sum_GREEN;
			(*result)[row * this->width + col]->blue = sum_BLUE;
			(*result)[row * this->width + col]->gray = sum_GRAY;
		}


	}

	return new IMG(this->width, this->height, result);
}

/**
 * @brief IMG::cross
 * @param pixel исходный массив пикселей
 * @param matrix матрица фильтр
 * @param div   делитель для матрицы фильтр
 * @param e     краевой эффект
 * @return      новый массив пикселей
 */
IMG* IMG::cross_GRAY(vector<vector<double> >& matrix, double div, edgeEffect e)
{
	vector<Pixel*>* pixel = this->list;


	//    Pixel **result =  new Pixel*[this->width * this->height];

	vector<Pixel*>* result = new vector<Pixel*>();
	for (int i = 0; i < this->width * this->height; i++)
		result->push_back(new Pixel(
			(*pixel)[i]->red,
			(*pixel)[i]->green,
			(*pixel)[i]->blue,
			(*pixel)[i]->alpha,
			(*pixel)[i]->gray
		));



	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int ku = matrix.size() / 2;
			int kv = matrix[0].size() / 2;
			double sum_GRAY = 0.0;
			double value_GRAY = 0.0;

			for (int u = -ku; u <= ku; u++) {
				for (int v = -kv; v <= kv; v++) {
					int x = row - u;
					int y = col - v;


					if (x < 0 || y < 0 || x >= this->height || y >= this->width) {

						if (x < 0)
							x = x + this->height;
						else if (x >= this->height)
							x = x - this->height;

						if (y < 0)
							y = y + this->width;
						else if (y >= this->width)
							y = y - this->width;



					}

					value_GRAY = (*pixel)[x * this->width + y]->gray;


					sum_GRAY += matrix[u + ku][v + kv] * value_GRAY;
				}
			}

			sum_GRAY *= div;
			(*result)[row * this->width + col]->gray = sum_GRAY;
		}


	}

	return new IMG(this->width, this->height, result);
}



IMG* IMG::normalize_COLOR()
{
	unsigned int start_time = clock();
	vector<Pixel*>* pixel = this->list;
	vector<Pixel*>* result = new vector<Pixel*>();
	for (int i = 0; i < this->width * this->height; i++) {
		result->push_back(new Pixel(
			(*pixel)[i]->red,
			(*pixel)[i]->green,
			(*pixel)[i]->blue,
			(*pixel)[i]->alpha,
			(*pixel)[i]->gray
		));
	}
	//Pixel **result =  new Pixel*[this->width * this->height];

	double max_RED = -99999999.0;
	double min_RED = 99999999.0;
	double max_GREEN = -99999999.0;
	double min_GREEN = 99999999.0;
	double max_BLUE = -99999999.0;
	double min_BLUE = 99999999.0;
	double max_GRAY = -99999999.0;
	double min_GRAY = 99999999.0;

	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;
			if (max_RED < (*pixel)[index]->red)
				max_RED = (*pixel)[index]->red;
			else if (min_RED > (*pixel)[index]->red)
				min_RED = (*pixel)[index]->red;

			if (max_GREEN < (*pixel)[index]->green)
				max_GREEN = (*pixel)[index]->green;
			else if (min_GREEN > (*pixel)[index]->green)
				min_GREEN = (*pixel)[index]->green;

			if (max_BLUE < (*pixel)[index]->blue)
				max_BLUE = (*pixel)[index]->blue;
			else if (min_BLUE > (*pixel)[index]->blue)
				min_BLUE = (*pixel)[index]->blue;

			if (max_GRAY < (*pixel)[index]->gray)
				max_GRAY = (*pixel)[index]->gray;
			else if (min_GRAY > (*pixel)[index]->gray)
				min_GRAY = (*pixel)[index]->gray;
		}
	}
	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;

			double tmp_RED = ((*pixel)[index]->red - min_RED) * ((1.0 - 0.0) / (max_RED - min_RED)) + 0.0;
			double tmp_GREEN = ((*pixel)[index]->green - min_GREEN) * ((1.0 - 0.0) / (max_GREEN - min_GREEN)) + 0.0;
			double tmp_BLUE = ((*pixel)[index]->blue - min_BLUE) * ((1.0 - 0.0) / (max_BLUE - min_BLUE)) + 0.0;
			double tmp_GRAY = ((*pixel)[index]->gray - min_GRAY) * ((1.0 - 0.0) / (max_GRAY - min_GRAY)) + 0.0;

			(*result)[index]->red = tmp_RED;
			(*result)[index]->green = tmp_GREEN;
			(*result)[index]->blue = tmp_BLUE;
			(*result)[index]->gray = tmp_GRAY;
		}
	}
	cout << "normalize : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result);
}

IMG* IMG::normalize_GRAY()
{
	unsigned int start_time = clock();
	vector<Pixel*>* pixel = this->list;
	vector<Pixel*>* result = new vector<Pixel*>();
	for (int i = 0; i < this->width * this->height; i++) {
		result->push_back(new Pixel(
			(*pixel)[i]->red,
			(*pixel)[i]->green,
			(*pixel)[i]->blue,
			(*pixel)[i]->alpha,
			(*pixel)[i]->gray
		));
	}

	double max_GRAY = -99999999.0;
	double min_GRAY = 99999999.0;

	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;

			if (max_GRAY < (*pixel)[index]->gray)
				max_GRAY = (*pixel)[index]->gray;
			else if (min_GRAY > (*pixel)[index]->gray)
				min_GRAY = (*pixel)[index]->gray;
		}
	}
	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;
			double tmp_GRAY = ((*pixel)[index]->gray - min_GRAY) * ((1.0 - 0.0) / (max_GRAY - min_GRAY)) + 0.0;
			(*result)[index]->gray = tmp_GRAY;
		}
	}
	cout << "normalize : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result);
}

/**
 * @brief IMG::sobelDerivativeX Оператор собеля по X
 * @param pixel массив пикселей
 * @param e Краевой эффект
 * @return Новый массив пикселей
 */
IMG* IMG::sobelDerivativeX(edgeEffect e)
{
	unsigned int start_time = clock();
	vector<vector<double>> matrix_sobolX({
		vector<double>({1, 0, -1}),
		vector<double>({2, 0, -2}),
		vector<double>({1, 0, -1})
		});

	//    Pixel **result_X = this->cross(pixel, matrix_sobolX, 1.0, e);
	IMG* result_X = this->cross_GRAY(matrix_sobolX, 1.0, e);
	//this->normalize(result_X);
	cout << "sobelDerivativeX : " << (clock() - start_time) / 1000.0 << "\n";
	return result_X;
}
/**
 * @brief IMG::sobelDerivativeY Оператор собеля по X
 * @param pixel массив пикселей
 * @param e Краевой эффект
 * @return Новый массив пикселей
 */
IMG* IMG::sobelDerivativeY(IMG::edgeEffect e)
{
	unsigned int start_time = clock();
	vector<vector<double>> matrix_sobolY({
		vector<double>({1, 2, 1}),
		vector<double>({0, 0, 0}),
		vector<double>({-1, -2, -1})
		});
	//    Pixel **result_Y = this->cross(pixel, matrix_sobolY, 1.0, e);
	IMG* result_Y = this->cross_GRAY(matrix_sobolY, 1.0, e);
	//this->normalize(result_Y);
	cout << "sobelDerivativeY : " << (clock() - start_time) / 1000.0 << "\n";
	return result_Y;
}

/**
 * @brief IMG::sobelGradient Вычисляем величину градиента
 * @param pixelX
 * @param pixelY
 * @return
 */
IMG* IMG::sobelGradient(IMG* imgX, IMG* imgY)
{
	unsigned int start_time = clock();
	vector<Pixel*>* pixelX = imgX->list;
	vector<Pixel*>* pixelY = imgY->list;
	vector<Pixel*>* result_XY = new vector<Pixel*>();

	for (int i = 0; i < this->width * this->height; i++)
		result_XY->push_back(NULL);

	for (int i = 0; i < this->height; i++) {
		for (int j = 0; j < this->width; j++) {
			int index = i * this->width + j;
			double val_X = (*pixelX)[index]->gray;
			double val_Y = (*pixelY)[index]->gray;
			double sq = sqrt(val_X * val_X + val_Y * val_Y);
			//sq = qMax(qMin(sq, 255.0), 0.0);
			(*result_XY)[index] = new Pixel(sq);
		}
	}
	cout << "sobelGradient : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result_XY);
}

IMG* IMG::gaussFilter(double sigma)
{
	unsigned int start_time = clock();
	vector<vector<double>> matrix_gauss;




	int size = (int)(3 * sigma);
	int halfSize = size / 2;
	double ss2 = 2 * sigma * sigma;
	double firstDrob = 1.0 / (M_PI * ss2);
	double test_sum = 0.0;
	for (int x = -halfSize; x <= halfSize; x++) {
		vector<double> tmp;
		for (int y = -halfSize; y <= halfSize; y++) {
			double gauss = firstDrob * exp(-(x * x + y * y) / ss2);
			tmp.push_back(gauss);
			test_sum += gauss;
		}
		matrix_gauss.push_back(tmp);
	}
	double test_sum_after = 0.0;
	for (int x = -halfSize; x <= halfSize; x++) {
		for (int y = -halfSize; y <= halfSize; y++) {
			matrix_gauss[x + halfSize][y + halfSize] /= test_sum;
			test_sum_after += matrix_gauss[x + halfSize][y + halfSize];
		}
	}

	//    Pixel **gauss_result  = this->cross(this->pixels, matrix_gauss, 1.0, IMG::edgeEffect::BLACK);
	IMG* gauss_result = this->cross_COLOR(matrix_gauss, 1.0, IMG::edgeEffect::BLACK);

	cout << "gaussFilter : " << (clock() - start_time) / 1000.0 << "\n";
	return gauss_result;
}

IMG* IMG::gaussFilter_separable(double sigma)
{
	vector<vector<double>> matrix_gauss;
	unsigned int start_time = clock();

	int size = (int)(3 * sigma);
	int halfSize = size / 2;
	double ss2 = 2 * sigma * sigma;
	double firstDrob = 1.0 / (M_PI * ss2);

	// Инициализируем первую строку фильтра
	vector<double> tmp;
	for (int x = -halfSize; x <= halfSize; x++) {
		double gauss = firstDrob * exp(-(x * x) / ss2);
		tmp.push_back(gauss);

	}
	matrix_gauss.push_back(tmp);


	//    Pixel **gauss_result_row;
	IMG* gauss_result_row;
	gauss_result_row = this->cross_COLOR(matrix_gauss, 1.0, IMG::edgeEffect::BLACK);


	// Инициализируем первый столбец фильтра

	// TODO почистить память matrix_gauss
	matrix_gauss.clear();
	for (int y = -halfSize; y <= halfSize; y++) {
		double gauss = firstDrob * exp(-(y * y) / ss2);
		vector<double> tmp;  // один элемент в строке
		tmp.push_back(gauss);
		matrix_gauss.push_back(tmp);
	}
	//    Pixel **gauss_result_col;
	IMG* gauss_result_col;
	gauss_result_col = gauss_result_row->cross_COLOR(matrix_gauss, 1.0, IMG::edgeEffect::BLACK);

	// TODO
	//this->deletePixels(gauss_result_row);

	cout << "gaussFilter : " << (clock() - start_time) / 1000.0 << "\n";
	return gauss_result_col;
}




Mat IMG::createImage_GRAY()
{
	vector<Pixel*>* pixel = this->list;

	int new_img_size_row = this->height;
	int new_img_size_col = this->width;
	Mat new_img = Mat::zeros(new_img_size_row, new_img_size_col, CV_8UC3);

	for (int row = 0; row < new_img.rows; row++) {
		for (int col = 0; col < new_img.cols; col++) {
			Vec3b& color = new_img.at<Vec3b>(row, col);
			color[2] = (*pixel)[row * this->width + col]->gray * 255.0;
			color[1] = (*pixel)[row * this->width + col]->gray * 255.0;
			color[0] = (*pixel)[row * this->width + col]->gray * 255.0;

		}
	}
	return new_img;
}

Mat IMG::createImage_COLOR()
{
	vector<Pixel*>* pixel = this->list;

	int new_img_size_row = this->height;
	int new_img_size_col = this->width;
	Mat new_img = Mat::zeros(new_img_size_row, new_img_size_col, CV_8UC3);

	for (int row = 0; row < new_img.rows; row++) {
		for (int col = 0; col < new_img.cols; col++) {
			Vec3b& color = new_img.at<Vec3b>(row, col);
			color[2] = (*pixel)[row * this->width + col]->red * 255.0;
			color[1] = (*pixel)[row * this->width + col]->green * 255.0;
			color[0] = (*pixel)[row * this->width + col]->blue * 255.0;
			int a = 2;
		}
	}
	return new_img;
}




IMG* IMG::downSample()
{
	int new_width = this->width / 2;
	int new_height = this->height / 2;
	vector<Pixel*>* new_pixels = new vector<Pixel*>();

	for (int row = 0; row < new_height; row++) {
		for (int col = 0; col < new_width; col++) {
			new_pixels->push_back(new Pixel(
				(*this->list)[row * 2 * this->width + col * 2]->red,
				(*this->list)[row * 2 * this->width + col * 2]->green,
				(*this->list)[row * 2 * this->width + col * 2]->blue,
				(*this->list)[row * 2 * this->width + col * 2]->alpha,
				(*this->list)[row * 2 * this->width + col * 2]->gray
			));
		}
	}
	return new IMG(new_width, new_height, new_pixels);
}

double IMG::getColor(int coordinate, IMG::COLOR color) {

	switch (color) {
	case IMG::COLOR::RED: {
		return (*this->list)[coordinate]->red;
	}
	case IMG::COLOR::GREEN: {
		return (*this->list)[coordinate]->green;
	}
	case IMG::COLOR::BLUE: {
		return (*this->list)[coordinate]->blue;
	}
	case IMG::COLOR::GRAY: {
		return (*this->list)[coordinate]->gray;
	}

	}
}