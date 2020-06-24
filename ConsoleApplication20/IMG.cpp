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
#include "ExWithCoordinate.h"

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

//vector<Pixel*>* IMG::getPixels() const
//{
//	return this->list;
//}




//void IMG::setPixels(vector<Pixel*>* value)
//{
//	int size = this->width * this->height;
//	//    for(int i=0; i < size; ++i)
//	//        delete this->pixels[i];
//	//    delete [] this->pixels;
//
//	list = value;
//}





// ������� ���������� ������ **Pixel, � ������� ������ ���� (������� ��� � ���������� [0...1])
void IMG::initImageDoubleArr()
{

	for (int row = 0; row < this->height; ++row) {
		for (int col = 0; col < this->width; ++col) {
			Vec3b& color = (this->image.at<Vec3b>(row, col));
			int blue = color[0];
			int green = color[1];
			int red = color[2];
			//int gray = ((double)red * 0.2125) + ((double)green * 0.7154) + ((double)blue * 0.0721);
			int gray = ((double)red * 0.213) + ((double)green * 0.715) + ((double)blue * 0.0722);

			//list->push_back(new Pixel((red / 255.0), (green / 255.0), (blue / 255.0), (gray / 255.0), gray / 255.0));

			this->pixels_red[row * this->width + col] = red / 255.0;
			this->pixels_green[row * this->width + col] = green / 255.0;
			this->pixels_blue[row * this->width + col] = blue / 255.0;
			this->pixels_gray[row * this->width + col] = gray / 255.0;
		}
	}
}






void IMG::deletePixels()
{
#ifdef CLEAR_MEMORY
	delete[] pixels_red;
	delete[] pixels_green;
	delete[] pixels_blue;
	delete[] pixels_gray;
	/*for (int i = 0; i < this->list->size(); ++i)
		delete (*list)[i];

	this->list->clear();
	this->list->shrink_to_fit();

	delete this->list;*/
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
 * @param pixel �������� ������ ��������
 * @param matrix ������� ������
 * @param div   �������� ��� ������� ������
 * @param e     ������� ������
 * @return      ����� ������ ��������
 */
IMG* IMG::cross_COLOR(vector<vector<double> >& matrix, double div, edgeEffect e)
{
	//vector<Pixel*>* pixel = this->list;


	//    Pixel **result =  new Pixel*[this->width * this->height];

	//vector<Pixel*>* result = new vector<Pixel*>();


	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}



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
					value_RED = this->pixels_red[x * this->width + y];
					value_GREEN = this->pixels_green[x * this->width + y];
					value_BLUE = this->pixels_blue[x * this->width + y];
					value_GRAY = this->pixels_gray[x * this->width + y];

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
			result_red[row * this->width + col] = sum_RED;
			result_green[row * this->width + col] = sum_GREEN;
			result_blue[row * this->width + col] = sum_BLUE;
			result_gray[row * this->width + col] = sum_GRAY;
		}


	}

	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

IMG* IMG::cross_COLOR(vector<ExWithCoordinate*>* extremumList, int windowSize, vector<vector<double> >& matrix, double div, edgeEffect e)
{
	//vector<Pixel*>* pixel = this->list;


	//    Pixel **result =  new Pixel*[this->width * this->height];

	//vector<Pixel*>* result = new vector<Pixel*>();


	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}


	for (int extremum_index = 0; extremum_index < extremumList->size(); extremum_index++) {
		ExWithCoordinate* extremum = extremumList->at(extremum_index);

		for (int yy = -windowSize; yy < windowSize; yy++) {
			for (int xx = -windowSize; xx < windowSize; xx++) {
				int row = extremum->row + yy;
				int col = extremum->col + xx;
				{
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
							value_RED = this->pixels_red[x * this->width + y];
							value_GREEN = this->pixels_green[x * this->width + y];
							value_BLUE = this->pixels_blue[x * this->width + y];
							value_GRAY = this->pixels_gray[x * this->width + y];

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
					result_red[row * this->width + col] = sum_RED;
					result_green[row * this->width + col] = sum_GREEN;
					result_blue[row * this->width + col] = sum_BLUE;
					result_gray[row * this->width + col] = sum_GRAY;
				}
			}
		}
	}


	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

/**
 * @brief IMG::cross
 * @param pixel �������� ������ ��������
 * @param matrix ������� ������
 * @param div   �������� ��� ������� ������
 * @param e     ������� ������
 * @return      ����� ������ ��������
 */
IMG* IMG::cross_GRAY(vector<vector<double> >& matrix, double div, edgeEffect e)
{
	//vector<Pixel*>* pixel = this->list;


	//    Pixel **result =  new Pixel*[this->width * this->height];

	//vector<Pixel*>* result = new vector<Pixel*>();

	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}

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

					value_GRAY = this->pixels_gray[x * this->width + y];


					sum_GRAY += matrix[u + ku][v + kv] * value_GRAY;
				}
			}

			sum_GRAY *= div;
			result_gray[row * this->width + col] = sum_GRAY;
		}


	}

	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

IMG* IMG::cross_GRAY(vector<ExWithCoordinate*>* extremumList, int windowSize, vector<vector<double> >& matrix, double div, edgeEffect e)
{
	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}

	for (int extremum_index = 0; extremum_index < extremumList->size(); extremum_index++) {
		ExWithCoordinate* extremum = extremumList->at(extremum_index);

		for (int yy = -windowSize; yy < windowSize; yy++) {
			for (int xx = -windowSize; xx < windowSize; xx++) {
				int row = extremum->row + yy;
				int col = extremum->col + xx;
				{
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

							value_GRAY = this->pixels_gray[x * this->width + y];


							sum_GRAY += matrix[u + ku][v + kv] * value_GRAY;
						}
					}

					sum_GRAY *= div;
					result_gray[row * this->width + col] = sum_GRAY;
				}
			}
		}
	}


	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

IMG* IMG::normalize_COLOR()
{
	unsigned int start_time = clock();

	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}

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
			if (max_RED < this->pixels_red[index])
				max_RED = this->pixels_red[index];
			else if (min_RED > this->pixels_red[index])
				min_RED = this->pixels_red[index];

			if (max_GREEN < this->pixels_green[index])
				max_GREEN = this->pixels_green[index];
			else if (min_GREEN > this->pixels_green[index])
				min_GREEN = this->pixels_green[index];

			if (max_BLUE < this->pixels_blue[index])
				max_BLUE = this->pixels_blue[index];
			else if (min_BLUE > this->pixels_blue[index])
				min_BLUE = this->pixels_blue[index];

			if (max_GRAY < this->pixels_gray[index])
				max_GRAY = this->pixels_gray[index];
			else if (min_GRAY > this->pixels_gray[index])
				min_GRAY = this->pixels_gray[index];
		}
	}
	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;

			double tmp_RED = (this->pixels_red[index] - min_RED) * ((1.0 - 0.0) / (max_RED - min_RED)) + 0.0;
			double tmp_GREEN = (this->pixels_green[index] - min_GREEN) * ((1.0 - 0.0) / (max_GREEN - min_GREEN)) + 0.0;
			double tmp_BLUE = (this->pixels_blue[index] - min_BLUE) * ((1.0 - 0.0) / (max_BLUE - min_BLUE)) + 0.0;
			double tmp_GRAY = (this->pixels_gray[index] - min_GRAY) * ((1.0 - 0.0) / (max_GRAY - min_GRAY)) + 0.0;

			result_red[index] = tmp_RED;
			result_green[index] = tmp_GREEN;
			result_blue[index] = tmp_BLUE;
			result_gray[index] = tmp_GRAY;
		}
	}
	cout << "normalize : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

IMG* IMG::normalize_GRAY()
{
	unsigned int start_time = clock();

	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}

	double max_GRAY = -99999999.0;
	double min_GRAY = 99999999.0;

	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;

			if (max_GRAY < this->pixels_gray[index])
				max_GRAY = this->pixels_gray[index];
			else if (min_GRAY > this->pixels_gray[index])
				min_GRAY = this->pixels_gray[index];
		}
	}
	for (int row = 0; row < this->height; row++) {
		for (int col = 0; col < this->width; col++) {
			int index = row * this->width + col;
			double tmp_GRAY = (this->pixels_gray[index] - min_GRAY) * ((1.0 - 0.0) / (max_GRAY - min_GRAY)) + 0.0;
			result_gray[index] = tmp_GRAY;
		}
	}
	cout << "normalize : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
}

/**
 * @brief IMG::sobelDerivativeX �������� ������ �� X
 * @param pixel ������ ��������
 * @param e ������� ������
 * @return ����� ������ ��������
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

IMG* IMG::sobelDerivativeX(vector<ExWithCoordinate*>* extremumList, int windowSize, edgeEffect e)
{
	unsigned int start_time = clock();
	vector<vector<double>> matrix_sobolX({
		vector<double>({1, 0, -1}),
		vector<double>({2, 0, -2}),
		vector<double>({1, 0, -1})
		});

	//    Pixel **result_X = this->cross(pixel, matrix_sobolX, 1.0, e);
	IMG* result_X = this->cross_GRAY(extremumList, windowSize, matrix_sobolX, 1.0, e);
	//this->normalize(result_X);
	cout << "sobelDerivativeX : " << (clock() - start_time) / 1000.0 << "\n";
	return result_X;
}











/**
 * @brief IMG::sobelDerivativeY �������� ������ �� X
 * @param pixel ������ ��������
 * @param e ������� ������
 * @return ����� ������ ��������
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

IMG* IMG::sobelDerivativeY(vector<ExWithCoordinate*>* extremumList, int windowSize, IMG::edgeEffect e)
{
	unsigned int start_time = clock();
	vector<vector<double>> matrix_sobolY({
		vector<double>({1, 2, 1}),
		vector<double>({0, 0, 0}),
		vector<double>({-1, -2, -1})
		});
	//    Pixel **result_Y = this->cross(pixel, matrix_sobolY, 1.0, e);
	IMG* result_Y = this->cross_GRAY(extremumList, windowSize,  matrix_sobolY, 1.0, e);
	//this->normalize(result_Y);
	cout << "sobelDerivativeY : " << (clock() - start_time) / 1000.0 << "\n";
	return result_Y;
}

/**
 * @brief IMG::sobelGradient ��������� �������� ���������
 * @param pixelX
 * @param pixelY
 * @return
 */
IMG* IMG::sobelGradient(IMG* imgX, IMG* imgY)
{
	unsigned int start_time = clock();


	double* result_red = new double[this->height * this->width];
	double* result_green = new double[this->height * this->width];
	double* result_blue = new double[this->height * this->width];
	double* result_gray = new double[this->height * this->width];

	for (int i = 0; i < this->width * this->height; i++) {
		result_red[i] = this->pixels_red[i];
		result_green[i] = this->pixels_green[i];
		result_blue[i] = this->pixels_blue[i];
		result_gray[i] = this->pixels_gray[i];
	}


	for (int i = 0; i < this->height; i++) {
		for (int j = 0; j < this->width; j++) {
			int index = i * this->width + j;
			double val_X = imgX->pixels_gray[index];
			double val_Y = imgY->pixels_gray[index];
			double sq = sqrt(val_X * val_X + val_Y * val_Y);

			result_red[index] = sq;
			result_green[index] = sq;
			result_blue[index] = sq;
			result_gray[index] = sq;
		}
	}
	cout << "sobelGradient : " << (clock() - start_time) / 1000.0 << "\n";
	return new IMG(this->width, this->height, result_red, result_green, result_blue, result_gray);
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

	// �������������� ������ ������ �������
	vector<double> tmp;
	for (int x = -halfSize; x <= halfSize; x++) {
		double gauss = firstDrob * exp(-(x * x) / ss2);
		tmp.push_back(gauss);

	}
	matrix_gauss.push_back(tmp);


	//    Pixel **gauss_result_row;
	IMG* gauss_result_row;
	gauss_result_row = this->cross_COLOR(matrix_gauss, 1.0, IMG::edgeEffect::BLACK);


	// �������������� ������ ������� �������

	// TODO ��������� ������ matrix_gauss
	matrix_gauss.clear();
	for (int y = -halfSize; y <= halfSize; y++) {
		double gauss = firstDrob * exp(-(y * y) / ss2);
		vector<double> tmp;  // ���� ������� � ������
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

IMG* IMG::gaussFilter_separable(vector<ExWithCoordinate*>* extremumList, int windowSize, double sigma)
{
	vector<vector<double>> matrix_gauss;
	unsigned int start_time = clock();

	int size = (int)(3 * sigma);
	int halfSize = size / 2;
	double ss2 = 2 * sigma * sigma;
	double firstDrob = 1.0 / (M_PI * ss2);

	// �������������� ������ ������ �������
	vector<double> tmp;
	for (int x = -halfSize; x <= halfSize; x++) {
		double gauss = firstDrob * exp(-(x * x) / ss2);
		tmp.push_back(gauss);

	}
	matrix_gauss.push_back(tmp);


	//    Pixel **gauss_result_row;
	IMG* gauss_result_row;
	gauss_result_row = this->cross_COLOR(extremumList, windowSize, matrix_gauss, 1.0, IMG::edgeEffect::BLACK);


	// �������������� ������ ������� �������

	// TODO ��������� ������ matrix_gauss
	matrix_gauss.clear();
	for (int y = -halfSize; y <= halfSize; y++) {
		double gauss = firstDrob * exp(-(y * y) / ss2);
		vector<double> tmp;  // ���� ������� � ������
		tmp.push_back(gauss);
		matrix_gauss.push_back(tmp);
	}
	//    Pixel **gauss_result_col;
	IMG* gauss_result_col;
	gauss_result_col = gauss_result_row->cross_COLOR(extremumList, windowSize, matrix_gauss, 1.0, IMG::edgeEffect::BLACK);

	// TODO
	//this->deletePixels(gauss_result_row);

	cout << "gaussFilter : " << (clock() - start_time) / 1000.0 << "\n";
	return gauss_result_col;
}


Mat IMG::createImage_GRAY()
{
	int new_img_size_row = this->height;
	int new_img_size_col = this->width;
	Mat new_img = Mat::zeros(new_img_size_row, new_img_size_col, CV_8UC3);

	for (int row = 0; row < new_img.rows; row++) {
		for (int col = 0; col < new_img.cols; col++) {
			Vec3b& color = new_img.at<Vec3b>(row, col);
			color[2] = this->pixels_gray[row * this->width + col] * 255.0;
			color[1] = this->pixels_gray[row * this->width + col] * 255.0;
			color[0] = this->pixels_gray[row * this->width + col] * 255.0;
		}
	}
	return new_img;
}

Mat IMG::createImage_COLOR()
{
	int new_img_size_row = this->height;
	int new_img_size_col = this->width;
	Mat new_img = Mat::zeros(new_img_size_row, new_img_size_col, CV_8UC3);

	for (int row = 0; row < new_img.rows; row++) {
		for (int col = 0; col < new_img.cols; col++) {
			Vec3b& color = new_img.at<Vec3b>(row, col);
			color[2] = this->pixels_red[row * this->width + col] * 255.0;
			color[1] = this->pixels_green[row * this->width + col] * 255.0;
			color[0] = this->pixels_blue[row * this->width + col] * 255.0;
			int a = 2;
		}
	}
	return new_img;
}




IMG* IMG::downSample()
{
	int new_width = this->width / 2;
	int new_height = this->height / 2;
	
	double* result_red = new double[new_height * new_width];
	double* result_green = new double[new_height * new_width];
	double* result_blue = new double[new_height * new_width];
	double* result_gray = new double[new_height * new_width];


	for (int row = 0; row < new_height; row++) {
		for (int col = 0; col < new_width; col++) {
			result_red[row * new_width + col] = this->pixels_red[row * 2 * this->width + col * 2];
			result_green[row * new_width + col] = this->pixels_green[row * 2 * this->width + col * 2];
			result_blue[row * new_width + col] = this->pixels_blue[row * 2 * this->width + col * 2];
			result_gray[row * new_width + col] = this->pixels_gray[row * 2 * this->width + col * 2];
		}
	}
	return new IMG(new_width, new_height, result_red, result_green, result_blue, result_gray);
}

double IMG::getColor(int coordinate, IMG::COLOR color) {

	switch (color) {
	case IMG::COLOR::RED: {
		return this->pixels_red[coordinate];
	}
	case IMG::COLOR::GREEN: {
		return this->pixels_green[coordinate];
	}
	case IMG::COLOR::BLUE: {
		return this->pixels_blue[coordinate];
	}
	case IMG::COLOR::GRAY: {
		return this->pixels_gray[coordinate];
	}

	}
}