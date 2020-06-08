#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <opencv2/opencv.hpp>
#include <windows.h>
#include <string>
#include <iostream>
#include <math.h>




#include "DEF.h"

#include "IMG.h"
#include "Octava.h"
#include "Pyramid_IMG.h"
//#include "Pixel.h"
#include "Moravec.h"
#include "HarrisPixel.h"
#include "Harris.h"
#include "Descriptors.h"
#include "Descriptors_turn.h"



using namespace std;
using namespace cv;

bool isMoravec = false;
bool isHarris = true;
bool isOctavi = false;
bool isSaveGray = false;
bool isSaveColor = false;

vector<Octava*>* testOctavi(IMG* img, string dir, double sigma0, double sigma1, int countOctav, int s) {

	double k = pow(2.0, 1.0 / (s - 1));
	double sigmaCurrent = sqrt(sigma1 * sigma1 - sigma0 * sigma0);


	// Считаем сигмы октав
	vector<double>* list_sigma = new vector<double>();
	list_sigma->push_back(sigma1);
	for (int i = 0; i < s - 1; i++) {
		list_sigma->push_back((*list_sigma)[i] * k);
	}

	// ДОСГЛАЖИВАЕМ
	IMG* img_first = img->gaussFilter_separable(sigmaCurrent);
	IMG* img_normalized_first = img_first->normalize_COLOR();
	img_first->deletePixels();

	img_normalized_first->saveImage_COLOR("c_досгладили.jpg", dir);


	vector<Octava*>* list_octava = new vector<Octava*>;


	double sigmaGlobal = sigma1;

	// считаем сигмы, для октав
	for (int octava_i = 0; octava_i < countOctav; octava_i++) {
		Octava* currentOctava = new Octava();
		//
		for (int i = 0; i < s; i++) {
			Pyramid_IMG* pyramid_img = new Pyramid_IMG();
			pyramid_img->sigma = (*list_sigma)[i];
			pyramid_img->layerNum = i;
			pyramid_img->octavaNum = octava_i;

			// Первая сигма в октаве равна последней сигме в предыдущей октаве
			if (i == 0)
				pyramid_img->globalSigma = sigmaGlobal;
			else
				pyramid_img->globalSigma = sigmaGlobal = sigmaGlobal * k;

			currentOctava->images->push_back(pyramid_img);
		}
		list_octava->push_back(currentOctava);
	}

	(*(*list_octava)[0]->images)[0]->img = img;


	for (int octava_i = 0; octava_i < countOctav; octava_i++) {
		Octava* currentOctava = (*list_octava)[octava_i];

		if ((*currentOctava->images)[0]->img == NULL) {
			Octava* prevOctava = (*list_octava)[octava_i - 1];

			Pyramid_IMG* lastPyramid = (*prevOctava->images)[prevOctava->images->size() - 1];

			IMG* firstImageInOctava = lastPyramid->img->downSample();

			(*currentOctava->images)[0]->img = firstImageInOctava;
		}
		//
		for (int i = 1; i < s; i++) {
			Pyramid_IMG* pyramid_prev = (*currentOctava->images)[i - 1];
			Pyramid_IMG* pyramid_current = (*currentOctava->images)[i];

			double sigma = sqrt(pyramid_current->sigma * pyramid_current->sigma -
				pyramid_prev->sigma * pyramid_prev->sigma);

			IMG* img_new = pyramid_prev->img->gaussFilter_separable(sigma);
			IMG* img_normalized_new = img_new->normalize_COLOR();

			img_new->deletePixels();

			pyramid_current->img = img_normalized_new;
		}
	}

	for (int i = 0; i < list_octava->size(); ++i) {
		vector<Pyramid_IMG*>* images = (*list_octava)[i]->images;
		for (int j = 0; j < images->size(); ++j) {
			Pyramid_IMG* pyramid = (*images)[j];
			pyramid->img->saveImage_COLOR(
				"c_" + to_string(pyramid->octavaNum) + "__" + to_string(pyramid->layerNum) + ".jpg",
				dir + "/octavi"
			);
		}
	}
	return list_octava;
}

double l_func(vector<Octava*>* list_octava, int x, int y, double sigma, IMG::COLOR color) {

	// Нужно найти ближайщий слой к заданной сигма
	Pyramid_IMG* needPyramid = (*(*list_octava)[0]->images)[0];

	for (Octava* octava : (*list_octava)) {
		for (Pyramid_IMG* pyramid : (*octava->images)) {
			// Если очередная пирамида ближе по сигма, чем ранее выбрананя
			if (abs(pyramid->globalSigma - sigma) < abs(needPyramid->globalSigma - sigma))
				needPyramid = pyramid;
		}
	}
	// В заданной пирамиде получаем значение пикселя
	// вычисляем координату, соответствующую текущей октаве
	int x_new = (int)(x / pow(2.0, needPyramid->octavaNum));
	int y_new = (int)(y / pow(2.0, needPyramid->octavaNum));

	return  needPyramid->img->getColor(y_new * needPyramid->img->width + x_new, color);
}

void lab3() {
	string dir = "C:/_img";
	string filename = "_Lena.jpg";

	IMG* img = new IMG(dir + "/" + filename);


	IMG* img_normalize = img->normalize_COLOR();

	if (isSaveColor)
		img_normalize->saveImage_COLOR("color.jpg", dir + "/result");

	if (isSaveGray)
		img_normalize->saveImage_GRAY("gray.jpg", dir + "/result");


	if (isOctavi) {
		vector<Octava*>* list_octava = testOctavi(img_normalize, dir + "/result", 0.5, 1.2, 5, 4);
		double kek = l_func(list_octava, 500, 500, 5.0, IMG::COLOR::RED);
		cout << "L = " << kek << "\n";
	}

	//

	if (isMoravec) {
		Moravec* moravec = new Moravec(img);
		IMG* moravec_result = moravec->calculate(5, 1);
		moravec_result->saveImage_COLOR("moravec_result.jpg", dir + "/result");
	}


	if (isHarris) {
		Harris* harris = new Harris(img);
		IMG* harris_result = harris->calculate(3, "_base", dir + "/result");
		delete harris_result;
	}
	if (isHarris) {
		IMG* img2 = new IMG(dir + "/" + "_Lena_turn.jpg");
		Harris* harris = new Harris(img2);
		IMG* harris_result = harris->calculate(3, "_turn", dir + "/result");
	}
}

void lab4(String firstName, String secondName, String dir, String resultName) {

	//////////////////////////////////////////////////////////////////////
	vector<pair<int, int>*>* first_Harris_indexUnicalPoint;
	IMG* first_img_Atan2;
	IMG* first_img_gradient;

	IMG* first_img = new IMG(dir + "/" + firstName);

	/*IMG* first_img_normalize = first_img->normalize_COLOR();
	delete first_img;
	first_img = first_img_normalize;*/

	Harris* first_harris = new Harris(first_img);
	IMG* first_calculated_harris = first_harris->calculate(3, "", dir + "/result");

	first_Harris_indexUnicalPoint = first_harris->getIndexUnicalPoint();
	first_img_Atan2 = first_harris->img_Atan2;
	first_img_gradient = first_harris->img_gradient;

	////////////////////////////////////////////////////////////////////////
	vector<pair<int, int>*>* second_Harris_indexUnicalPoint;
	IMG* second_img_Atan2;
	IMG* second_img_gradient;

	IMG* second_img = new IMG(dir + "/" +  secondName);

	/*IMG* second_img_normalize = second_img->normalize_COLOR();
	delete second_img;
	second_img = second_img_normalize;*/

	Harris* second_harris = new Harris(second_img);
	IMG* second_calculated_harris = second_harris->calculate(3, "_turn", dir + "/result");

	second_Harris_indexUnicalPoint = second_harris->getIndexUnicalPoint();
	second_img_Atan2 = second_harris->img_Atan2;
	second_img_gradient = second_harris->img_gradient;


	Descriptors* descriptors_first =
		new Descriptors(first_Harris_indexUnicalPoint, first_img_Atan2, first_img_gradient);

	Descriptors* descriptors_second =
		new Descriptors(second_Harris_indexUnicalPoint, second_img_Atan2, second_img_gradient);

	//пары точек, для линий
	vector<pair<Descriptor*, Descriptor*>*>* pairs =
		Descriptors::createPairs(descriptors_first, descriptors_second);


	// совмещенные картинки на одну
	IMG* resultImg = Descriptors::createDemoImg(first_img, second_img);

	// Рисуем линии на совмещенной картинке
	IMG* img_drawedLine = Descriptors::drawLine(first_img, second_img, resultImg, pairs);
	// нормализуем
	IMG* img_drawedLine_normalize = img_drawedLine->normalize_COLOR();
	// сохраняем
	img_drawedLine_normalize->saveImage_COLOR(resultName, dir + "/result");

#ifdef CLEAR_MEMORY
	delete resultImg;
	delete img_drawedLine;
	delete img_drawedLine_normalize;


	delete first_harris;
	delete second_harris;

	delete descriptors_first;
	delete descriptors_second;

	delete first_img;
	delete second_img;
#endif // CLEAR_MEMORY

  	cout << "";
}

void lab5(String dir, String firstName, String secondName, String resultName, double sigma){
	//////////////////////////////////////////////////////////////////////
	vector<pair<int, int>*>* first_Harris_indexUnicalPoint;
	IMG* first_img_Atan2;
	IMG* first_img_gradient;

	IMG* first_img = new IMG(dir + "/" + firstName);
	first_img = first_img->gaussFilter_separable(sigma);
	first_img = first_img->normalize_COLOR();

	/*IMG* first_img_normalize = first_img->normalize_COLOR();
	delete first_img;
	first_img = first_img_normalize;*/

	Harris* first_harris = new Harris(first_img);
	IMG* first_calculated_harris = first_harris->calculate(3, "", dir + "/result");

	first_Harris_indexUnicalPoint = first_harris->getIndexUnicalPoint();
	first_img_Atan2 = first_harris->img_Atan2;
	first_img_gradient = first_harris->img_gradient;

	////////////////////////////////////////////////////////////////////////
	vector<pair<int, int>*>* second_Harris_indexUnicalPoint;
	IMG* second_img_Atan2;
	IMG* second_img_gradient;

	IMG* second_img = new IMG(dir + "/" + secondName);
	second_img = second_img->gaussFilter_separable(sigma);
	second_img = second_img->normalize_COLOR();
	/*IMG* second_img_normalize = second_img->normalize_COLOR();
	delete second_img;
	second_img = second_img_normalize;*/

	Harris* second_harris = new Harris(second_img);
	IMG* second_calculated_harris = second_harris->calculate(3, "_turn", dir + "/result");

	second_Harris_indexUnicalPoint = second_harris->getIndexUnicalPoint();
	second_img_Atan2 = second_harris->img_Atan2;
	second_img_gradient = second_harris->img_gradient;

	Descriptors_turn* descriptors_first =
		new Descriptors_turn(first_Harris_indexUnicalPoint, first_img_Atan2, first_img_gradient, 0);

	Descriptors_turn* descriptors_second =
		new Descriptors_turn(second_Harris_indexUnicalPoint, second_img_Atan2, second_img_gradient, 0);

	//пары точек, для линий
	vector<pair<Descriptor*, Descriptor*>*>* pairs =
		Descriptors_turn::createPairs(descriptors_first, descriptors_second);


	// совмещенные картинки на одну
	IMG* resultImg = Descriptors::createDemoImg(first_img, second_img);

	// Рисуем линии на совмещенной картинке
	IMG* img_drawedLine = Descriptors::drawLine(first_img, second_img, resultImg, pairs);
	// нормализуем
	IMG* img_drawedLine_normalize = img_drawedLine->normalize_COLOR();
	// сохраняем
	img_drawedLine_normalize->saveImage_COLOR(resultName, dir + "/result");

#ifdef CLEAR_MEMORY
	delete resultImg;
	delete img_drawedLine;
	delete img_drawedLine_normalize;


	delete first_harris;
	delete second_harris;

	delete descriptors_first;
	delete descriptors_second;

	delete first_img;
	delete second_img;
#endif // CLEAR_MEMORY

	cout << "";
}

int main(int argc, char** argv)
{
	unsigned int start_time = clock();
	double test = cos(360);

	string dir = "C:/_img/";
	String _1 = "lenka_1.jpg";
	String _2 = "lenka_2(2).jpg";

	//lab4(_1, _2, dir, "line.jpg");

	lab5(dir, "lenka_1.jpg", "lenka_turn90.jpg", "line.jpg", 2.0);

	cout << "FULL TIME : " << (clock() - start_time) / 1000.0 << "\n\n\n";

	return 0;
}








//Mat img_tmp = imread(dir + "/" + filename);
	//Mat* img = &img_tmp;

	//Vec3b intensity = img->at<Vec3b>(0, 0);
	//uchar blue = intensity.val[0];
	//uchar green = intensity.val[1];
	//uchar red = intensity.val[2];


	//cout << "Width : " << img->cols << endl;
	//cout << "Height: " << img->rows << endl;
	//cout << (int)red << "    " << (int)green << "    " << (int)blue << "\n";

	////for (int row = 0; row < img.rows; row++) {
	////	for (int col = 0; col < img.cols; col++) {
	////		Vec3b & color = img.at<Vec3b>(row, col);
	////		int blue = color[0];
	////		int green = color[1];
	////		int red = color[2];
	////		int gray = ((double)red * 0.2125) + ((double)green * 0.7154) + ((double)blue * 0.0721);
	////		color[0] = color[1] = color[2] = gray;
	////	}
	////}



	////imwrite(dir + "/" + "result.jpg", img);


	//int new_img_size_row = 1377;
	//int new_img_size_col = 1377;
	//Mat *new_img = new Mat(new_img_size_row, new_img_size_col, CV_8UC3, Scalar(100, 100, 100));

	//for (int row = 0; row < new_img->rows; row++) {
	//	for (int col = 0; col < new_img->cols; col++) {
	//		Vec3b & color = new_img->at<Vec3b>(row, col);
	//		color[0] = rand() % 255;
	//		color[1] = rand() % 255;
	//		color[2] = rand() % 255;

	//	}
	//}

	//imwrite(dir + "/" + "random.jpg", *new_img);

	////namedWindow("image", WINDOW_NORMAL);
	////imshow("image", img);
	////waitKey(0);