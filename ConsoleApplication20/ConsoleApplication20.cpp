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
#include "DoG_IMG.h"
#include "Lab6_Descriptors_turn.h"
#include "Lab6_Harris.h"

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

	(*(*list_octava)[0]->images)[0]->img = img_normalized_first;


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

	/*for (int i = 0; i < list_octava->size(); ++i) {
		vector<Pyramid_IMG*>* images = (*list_octava)[i]->images;
		for (int j = 0; j < images->size(); ++j) {
			Pyramid_IMG* pyramid = (*images)[j];
			pyramid->img->saveImage_COLOR(
				"c_" + to_string(pyramid->octavaNum) + "__" + to_string(pyramid->layerNum) + ".jpg",
				dir + "/octavi"
			);
		}
	}*/
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
	vector<pair<InterestingPoint*, InterestingPoint*>*>* pairs =
		Descriptors_turn::createPairs(descriptors_first, descriptors_second);


	// совмещенные картинки на одну
	IMG* resultImg = Descriptors_turn::createDemoImg(first_img, second_img);

	// Рисуем линии на совмещенной картинке
	IMG* img_drawedLine = Descriptors_turn::drawLine(first_img, second_img, resultImg, pairs);
	// нормализуем
	IMG* img_drawedLine_normalize = img_drawedLine->normalize_COLOR();
	// сохраняем
	img_drawedLine_normalize->saveImage_COLOR(resultName, dir + "/result");


	// Еще нарисуем направления каждой из точек
	{
		IMG* imgFirstArrows = descriptors_first->drawArrows(first_img);
		IMG* imgFirstArrows_normalized = imgFirstArrows->normalize_COLOR();
		imgFirstArrows_normalized->saveImage_COLOR("Arrows_first.jpg",dir + "/result");

		IMG* imgSecondArrows = descriptors_second->drawArrows(second_img);
		IMG* imgSecondArrows_normalized = imgSecondArrows->normalize_COLOR();
		imgSecondArrows_normalized->saveImage_COLOR("Arrows_second.jpg" , dir + "/result");

		#ifdef CLEAR_MEMORY
		delete imgFirstArrows;
		delete imgFirstArrows_normalized;

		delete imgSecondArrows;
		delete imgSecondArrows_normalized;
		#endif // CLEAR_MEMORY
	}

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


vector<double>* findMaxMin(IMG* img, int r, int c, boolean isCheckCenter) {
	vector<double>* result = new vector<double>;
	double max = img->getGrayWithEdge(r - 1, c - 1);
	double min = img->getGrayWithEdge(r - 1, c - 1);
	for (int row = r - 1; row < r + 1; row++) {
		for (int col = c - 1; col < c + 1; col++) {
			// Если это центр, и нам надо его проверить
			if (r == row && c == col) {
				if (!isCheckCenter) {
					continue;
				}
			}
			if (max < img->getGrayWithEdge(row, col)) {
				max = img->getGrayWithEdge(row, col);
			}
			if (min > img->getGrayWithEdge(row, col)) {
				min = img->getGrayWithEdge(row, col);
			}

		}
	}
	result->push_back(max);
	result->push_back(min);
	return result;
}

vector<Octava*>* Lab6Calculate(IMG* img_first, int octav) {

	double EXTREMUM_LEVEL = 0.03;

	vector<Octava*>* octavas_first = testOctavi(img_first, "C:/_img/Lab6/result", 0.5, 1.6, octav, 5);

	for (int octavas_first_index = 0; octavas_first_index < octavas_first->size(); octavas_first_index++) {
		Octava* octava = octavas_first->at(octavas_first_index);

		for (int i = 0; i < octava->images->size() - 1; i++) {
			Pyramid_IMG* pyramid_first = octava->images->at(i);
			//                pyramid_first.img = pyramid_first.img.normalize_COLOR();
			Pyramid_IMG* pyramid_second = octava->images->at(i + 1);
			//                pyramid_second.img = pyramid_second.img.normalize_COLOR();

			IMG* result = pyramid_first->img->copy();
			for (int row = 0; row < pyramid_first->img->height; row++) {
				for (int col = 0; col < pyramid_first->img->width; col++) {
					double red =
						pyramid_second->img->getRedWithEdge(row, col) -
						pyramid_first->img->getRedWithEdge(row, col);

					result->pixels_red[row * result->width + col] = red;

					double green =
						pyramid_second->img->getGreenWithEdge(row, col) -
						pyramid_first->img->getGreenWithEdge(row, col);
					result->pixels_green[row * result->width + col] = green;

					double blue =
						pyramid_second->img->getBlueWithEdge(row, col) -
						pyramid_first->img->getBlueWithEdge(row, col);

					result->pixels_blue[row * result->width + col] = blue;

					double gray =
						pyramid_second->img->getGrayWithEdge(row, col) -
						pyramid_first->img->getGrayWithEdge(row, col);

					result->pixels_gray[row * result->width + col] = gray;
				}
			}
			DoG_IMG* dog_result = new DoG_IMG();
			dog_result->img = result;

			octava->dog->push_back(dog_result);
		}
	}


	// findExtremum
	for (int octavas_first_index = 0; octavas_first_index < octavas_first->size(); octavas_first_index++) {
		Octava* octava = octavas_first->at(octavas_first_index);


		for (int d = 1; d < octava->dog->size() - 1; d++) {
			DoG_IMG* prev_pyramid = octava->dog->at(d - 1);
			DoG_IMG* pyramid = octava->dog->at(d);
			DoG_IMG* next_pyramid = octava->dog->at(d + 1);
			int countExtremum = 0;

			vector<ExWithCoordinate*>* extremumList = new vector<ExWithCoordinate*>;
			for (int row = 1; row < pyramid->img->height - 1; row++) {
				for (int col = 1; col < pyramid->img->width - 1; col++) {
					//					Pixels pixel = pyramid.img.getPixelWithEdge(row, col);
					double gray = pyramid->img->getGrayWithEdge(row, col);
					double currentValue = gray;

					if (currentValue < EXTREMUM_LEVEL)
						continue;

					// Находим максимум на текущем, и двух соседних dog
					vector<double>* prev_maxMin =
						findMaxMin(prev_pyramid->img, row, col, true);

					vector<double>* maxMin =
						findMaxMin(pyramid->img, row, col, false);

					vector<double>* next_maxMin =
						findMaxMin(next_pyramid->img, row, col, true);

					double max = max(prev_maxMin->at(0), maxMin->at(0));
					max = max(max, next_maxMin->at(0));

					double min = min(prev_maxMin->at(1), maxMin->at(1));
					min = min(min, next_maxMin->at(1));


					if (currentValue < min) {
						countExtremum++;
						extremumList->push_back(new ExWithCoordinate(row, col, abs(currentValue)));
					}
					if (currentValue > max) {
						countExtremum++;
						extremumList->push_back(new ExWithCoordinate(row, col, abs(currentValue)));
					}

				}
			}

			// сортируем по УБЫВАНИЮ 
			//vector<ExWithCoordinate*>* extremumList = new vector<ExWithCoordinate*>;
			sort(extremumList->begin(), extremumList->end(),
				[](const ExWithCoordinate* first, const ExWithCoordinate* second) {
					return first->extremum > second->extremum;
				});

			cout << "наибольшие по модулю экстремумы (20шт)" << endl;
			cout << "октава # " + to_string(octava->images->at(d)->octavaNum) << endl;
			cout << "dog # " + to_string(d) << endl;
			for (int i = 0; i < min(20, extremumList->size()); i++) {
				cout << "#" + to_string(i) + "\t" + to_string(extremumList->at(i)->extremum) + "\t" + "[ " +
					to_string(extremumList->at(i)->row) + " ; " + to_string(extremumList->at(i)->col) + " ]" << endl;
			}
			cout << endl;
			pyramid->extremumCoordinates = extremumList;
		}
	}

	// saveDog
	//for (int octava_index = 0; octava_index < octavas_first->size(); octava_index++) {
	//	Octava* octava = octavas_first->at(octava_index);

	//	for (int i = 0; i < octava->dog->size(); i++) {
	//		DoG_IMG* image = octava->dog->at(i);
	//		Pyramid_IMG* pyramid_img = octava->images->at(i);
	//		IMG* tmp = image->img->normalize_GRAY();
	//		//tmp->saveImage_GRAY(getPath(dir, "result", "dog", pyramid_img.octavaNum + "_" + pyramid_img.layerNum + ".jpg"));
	//		tmp->saveImage_GRAY(to_string(pyramid_img->octavaNum) + "_" + to_string(pyramid_img->layerNum) + ".jpg","C:/_img/Lab6/result/dog");
	//	}
	//}

	//// saveOctav
	//for (int octava_index = 0; octava_index < octavas_first->size(); octava_index++) {
	//	Octava* octava = octavas_first->at(octava_index);
	//	for (int image_index = 0; image_index < octava->images->size(); image_index++) {
	//		Pyramid_IMG* image = octava->images->at(image_index);
	//		if(image == NULL)
	//			continue;
	//		IMG* tmp = image->img->normalize_COLOR();
	//		tmp->saveImage_COLOR(to_string(image->octavaNum) + "_" + to_string(image->layerNum) + ".jpg", "C:/_img/Lab6/result/octavi");
	//	}
	//}

	// Считаем харриса только для точек

	for (int octava_index = 0; octava_index < octavas_first->size(); octava_index++) {
		Octava* octava = octavas_first->at(octava_index);

		for (int i = 1; i < octava->dog->size() - 1; i++) {
			DoG_IMG* doG_img = octava->dog->at(i);
			Pyramid_IMG* pyramid_img = octava->images->at(i);
			Lab6_Harris* harris = new Lab6_Harris(pyramid_img->img);
			harris->calculate(doG_img->extremumCoordinates, 3, "_1", "");

			pyramid_img->imgHarrisPart = harris;
			cout << "";
		}
	}


	// Ищем дескрипторы для точек
	for (int octava_index = 0; octava_index < octavas_first->size(); octava_index++) {
		Octava* octava = octavas_first->at(octava_index);

		for (int i = 1; i < octava->dog->size() - 1; i++) {
			Pyramid_IMG* pyramid_img = octava->images->at(i);
			DoG_IMG* doG_img = octava->dog->at(i);

			vector<pair<int, int>*>* indexUnicalPoint_anms = pyramid_img->imgHarrisPart->getIndexUnicalPoint_ANMS();

			IMG* img_atan2_normalized = pyramid_img->imgHarrisPart->img_Atan2_normalized;
			IMG* img_gradient = pyramid_img->imgHarrisPart->img_gradient;

			double gridSizeFactor = (pyramid_img->sigma / 1.6) * 16;
			double size = gridSizeFactor / 4.0;
			int sizeInt = (int)round(size);

			Lab6_Descriptors_turn* lab6_descriptors_turn =
				new Lab6_Descriptors_turn(indexUnicalPoint_anms, img_atan2_normalized, img_gradient, 0);

			lab6_descriptors_turn->setSizeRegion(sizeInt);
			lab6_descriptors_turn->start();

			pyramid_img->descriptors = lab6_descriptors_turn;

			cout << endl;
		}
	}

	// сколько точек всего получилось
	int countInterestingPointFirst = 0;
	for (int octava_index = 0; octava_index < octavas_first->size(); octava_index++) {
		Octava* octava = octavas_first->at(octava_index);

		for (int image_index = 0; image_index < octava->images->size(); image_index++) {
			Pyramid_IMG* image = octava->images->at(image_index);

			if (image->descriptors == NULL)
				continue;
			vector<InterestingPoint*>* interestingPointList = image->descriptors->getInterestingPointList();
			int size = interestingPointList->size();
			countInterestingPointFirst += size;
		}
	}


	//            // saveDog
	//            for (Octava octava : octavas_first) {
	//                for (int i = 0; i < octava.dog.size(); i++) {
	//                    DoG_IMG image = octava.dog.get(i);
	//                    Pyramid_IMG pyramid_img = octava.images.get(i);
	//                    final IMG tmp = image.img.normalize_GRAY();
	//                    tmp.saveToFile_GRAY(getPath(dir, "result", "dog", pyramid_img.octavaNum + "_" + pyramid_img.layerNum + ".jpg"));
	//                }
	//            }
	//
	//            // saveOctav
	//            for (Octava octava : octavas_first) {
	//                for (Pyramid_IMG image : octava.images) {
	//                    final IMG tmp = image.img.normalize_COLOR();
	//                    tmp.saveToFile_COLOR(getPath(dir, "result", "octavi", image.octavaNum + "_" + image.layerNum + ".jpg"));
	//                }
	//            }

	return octavas_first;
}

void Lab6() {
	String dir = "C:/_img/Lab6";
	String firstPath = dir + "/" + "butterfly(0.6).jpg";
	String secondPath = dir + "/" + "butterfly(0.3).jpg";

	//        String dir = "images_source/Lab6";
	//        String firstPath = "images_source" + "/" + "lenka_turn.jpg";
	//        String secondPath = dir + "/" + "3.png";

	IMG* img_first = new IMG(firstPath);
	IMG* img_second = new IMG(secondPath);

	img_first = img_first->normalize_COLOR();
	img_second = img_second->normalize_COLOR();

	vector<Octava*>* octavas_first = Lab6Calculate(img_first, 3);

	//        // saveDog
	//        for (Octava octava : octavas_first) {
	//            for (int i = 0; i < octava.dog.size(); i++) {
	//                DoG_IMG doG_img = octava.dog.get(i);
	//                Pyramid_IMG image = octava.images.get(i);
	//                final IMG tmp = doG_img.img.normalize_GRAY();
	//                tmp.saveToFile_GRAY(getPath(dir, "result", "dog", image.octavaNum + "_" + image.layerNum + ".jpg"));
	//            }
	//        }
	//        // saveOctav
	//        for (Octava octava : octavas_first) {
	//            for (Pyramid_IMG image : octava.images) {
	//                final IMG tmp = image.img.normalize_COLOR();
	//                tmp.saveToFile_COLOR(getPath(dir, "result", "octavi", image.octavaNum + "_" + image.layerNum + ".jpg"));
	//            }
	//        }
			// saveDog CIRCLE
	for (int octava_first_index = 0; octava_first_index < octavas_first->size(); octava_first_index++) {
		Octava* octava = octavas_first->at(octava_first_index);

		for (int i = 1; i < octava->dog->size() - 1; i++) {
			DoG_IMG* doG_img = octava->dog->at(i);
			Pyramid_IMG* image = octava->images->at(i);
			IMG* tmp = doG_img->img->normalize_GRAY();


			vector<InterestingPoint*>* interestingPointList =
				image->descriptors->getInterestingPointList();

			double v = image->sigma / 1.6 * 16;
			v = round(v / 4);
			v = v * 4;
			double r = v / 2;
			vector<vector<int>*>* tmpFullList = new vector<vector<int>*>;
			for (int interestingPointList_index = 0; interestingPointList_index < interestingPointList->size(); interestingPointList_index++) {
				InterestingPoint* interestingPoint = interestingPointList->at(interestingPointList_index);

				vector<int>* tmpList = new vector<int>;
				tmpList->push_back(interestingPoint->row);
				tmpList->push_back(interestingPoint->col);
				tmpList->push_back((int)r);
				tmpFullList->push_back(tmpList);
			}

			//IMG* img_circle = Lab6_Descriptors_turn::drawCircle_gray(tmp->copy(), tmpFullList);
			//img_circle = img_circle->normalize_COLOR();
			//img_circle->saveImage_COLOR( to_string(image->octavaNum) + "_" + to_string(image->layerNum) + "_circle" + ".jpg",
			//	dir + "/" + "result" + "/" + "dog" + "/" );
		}
	}

	// row // col // radius
	vector<vector<int>*>* circleListALL = new vector<vector<int>*>;

	for (int q = 0; q < octavas_first->size(); q++) {
		vector<vector<int>*>* circleList = new vector<vector<int>*>;

		for (int image_index = 0; image_index < octavas_first->at(q)->images->size(); image_index++) {
			Pyramid_IMG* image = octavas_first->at(q)->images->at(image_index);

			if (image->descriptors == NULL)
				continue;

			vector<InterestingPoint*>* interestingPointList =
				image->descriptors->getInterestingPointList();

			double v = image->globalSigma / 1.6 * 16;
			v = round(v / 4);
			v = v * 4;
			double r = v / 2;
			for (int interestingPoint_index = 0; interestingPoint_index < interestingPointList->size(); interestingPoint_index++) {
				InterestingPoint* interestingPoint = interestingPointList->at(interestingPoint_index);
		

				vector<int>* tmpList = new vector<int>;
				tmpList->push_back(interestingPoint->row * (int)pow(2, image->octavaNum));
				tmpList->push_back(interestingPoint->col * (int)pow(2, image->octavaNum));
				tmpList->push_back((int)r);
				circleList->push_back(tmpList);
			}
		}

		//IMG* img_circle = Lab6_Descriptors_turn::drawCircle(img_first->copy(), circleList);

		//img_circle = img_circle->normalize_COLOR();
		//img_circle->saveImage_COLOR("circles_" + to_string(q) + ".jpg", dir + "/result");

		for(int w = 0 ; w < circleList->size(); w++)
			circleListALL->push_back(circleList->at(w));
	}

	//IMG* img_circle = Lab6_Descriptors_turn::drawCircle(img_first->copy(), circleListALL);
	//img_circle = img_circle->normalize_COLOR();
	//img_circle->saveImage_COLOR("circles_ALL.jpg", dir + "/result");



	vector<Octava*>* octavas_second = Lab6Calculate(img_second, 3);


	// совмещенные картинки на одну
	IMG* resultImg = Lab6_Descriptors_turn::createDemoImg(img_first, img_second);

	vector<InterestingPoint*>* interestingPointList_first = new vector<InterestingPoint*>;
	vector<InterestingPoint*>* interestingPointList_second = new vector<InterestingPoint*>;

	for (int octavas_first_index = 0; octavas_first_index < octavas_first->size(); octavas_first_index++) {
		Octava* octava = octavas_first->at(octavas_first_index);

		for (int image_first_index = 0; image_first_index < octava->images->size(); image_first_index++) {
			Pyramid_IMG* image_first = octava->images->at(image_first_index);

			Lab6_Descriptors_turn* descriptors_first = image_first->descriptors;

			if (descriptors_first == NULL)
				continue;

			int octavaNum = image_first->octavaNum;
			for (int interestingPoint_index = 0; interestingPoint_index < descriptors_first->getInterestingPointList()->size(); interestingPoint_index++) {
				InterestingPoint* interestingPoint = descriptors_first->getInterestingPointList()->at(interestingPoint_index);

				int realRow = (int)pow(2, octavaNum) * interestingPoint->row;
				int realCol = (int)pow(2, octavaNum) * interestingPoint->col;

				interestingPoint->realRow = realRow;
				interestingPoint->realCol = realCol;
			}
			for (int w = 0; w < descriptors_first->getInterestingPointList()->size(); w++) {
				interestingPointList_first->push_back(descriptors_first->getInterestingPointList()->at(w));
			}
		}
	}

	for (int octavas_second_index = 0; octavas_second_index < octavas_second->size(); octavas_second_index++) {
		Octava* octava = octavas_second->at(octavas_second_index);
		for (int image_second_index = 0; image_second_index < octava->images->size(); image_second_index++) {
			Pyramid_IMG* image_second = octava->images->at(image_second_index);

			Lab6_Descriptors_turn* descriptors_second = image_second->descriptors;
			if (descriptors_second == NULL)
				continue;

			int octavaNum = image_second->octavaNum;
			for (int interestingPoint_index = 0; interestingPoint_index < descriptors_second->getInterestingPointList()->size(); interestingPoint_index++) {
				InterestingPoint* interestingPoint = descriptors_second->getInterestingPointList()->at(interestingPoint_index);
			
				int realRow = (octavaNum + 1) * interestingPoint->row;
				int realCol = (octavaNum + 1) * interestingPoint->col;

				interestingPoint->realRow = realRow;
				interestingPoint->realCol = realCol;
			}
			for (int w = 0; w < descriptors_second->getInterestingPointList()->size(); w++) {
				interestingPointList_second->push_back(descriptors_second->getInterestingPointList()->at(w));
			}
		}
	}

	// пары точек, для линий
	// сравниваем дескрипторы первого и второго изображения, и находим наилучшие соответствия
	vector<pair<InterestingPoint*, InterestingPoint*>*>* pairs =
		Lab6_Descriptors_turn::createPairs(interestingPointList_first, interestingPointList_second);

	// Визуализируем линиями пары точек первого и второго изображения
	resultImg = Lab6_Descriptors_turn::drawLine(img_first, img_second, resultImg, pairs);

	// сохраняем визуализауцю
	resultImg = resultImg->normalize_COLOR();
	resultImg->saveImage_COLOR("lines.jpg", dir + "/result");

	//        // Еще нарисуем направления каждой из точек
	//        {
	//            final IMG imgFirstArrows = descriptors_first.drawArrows(first_img);
	//            imgFirstArrows.normalize_COLOR().saveToFile_COLOR("images/descriptors/Arrows_first.jpg");
	//
	//            final IMG imgSecondArrows = descriptors_second.drawArrows(second_img);
	//            imgSecondArrows.normalize_COLOR().saveToFile_COLOR("images/descriptors/Arrows_second.jpg");
	//        }


}






int main(int argc, char** argv)
{
	srand(time(0));
	unsigned int start_time = clock();
	double test = cos(360);

	Lab6();
	//string dir = "C:/_img/";
	//String _1 = "lenka_1.jpg";
	//String _2 = "lenka_2(2).jpg";

	////lab4(_1, _2, dir, "line.jpg");

	//lab5(dir, "Descriptors_turn/0.jpg", "Descriptors_turn/30.jpg", "line.jpg", 1.2);

	//cout << "FULL TIME : " << (clock() - start_time) / 1000.0 << "\n\n\n";

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