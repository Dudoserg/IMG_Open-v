#pragma once
#include "DEF.h"
#include "IMG.h"
#include "HarrisPixel.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;


class Lab6_Harris
{
public:
	IMG* img;
	double LAB6_HARRIS_LEVEL = 0.01;
	// полуразмер точки
	int POINT_SIZE = 2;

	// Полуразмер окна для поиска локального максмума
	int POINT_LOCAL_SIZE = 3;

	// коилчество уникальных точек

	int POINT_COUNT_Lab6_Harris = 200;

	int POINT_COUNT_TOP_POWERFUL = 200;
	int POINT_COUNT_TOP_ANMS = 200;

	double TOP_X_LEVEL = 0.95;

	vector<pair<int, int>*>* indexUnicalPoint = new vector<pair<int, int>*>;

	vector<pair<int, int>*>* getIndexUnicalPoint_ANMS() {
		return this->indexUnicalPoint;
	}


	///////////////////////
	IMG* img_Sobol_X;
	IMG* img_normalized_Sobol_X;

	IMG* img_Sobol_Y;
	IMG* img_normalized_Sobol_Y;



	IMG* img_Atan2;
	IMG* img_Atan2_normalized;
	IMG* img_gradient;


	Lab6_Harris(IMG* img) {
		this->img = img;
	};
	~Lab6_Harris() {
#ifdef CLEAR_MEMORY
		delete img_Atan2;
		delete img_Atan2_normalized;
		delete img_gradient;

		for (int i = 0; i < indexUnicalPoint->size(); i++) {
			delete indexUnicalPoint->at(i);
		}
		indexUnicalPoint->clear();
		indexUnicalPoint->shrink_to_fit();
		delete indexUnicalPoint;
#endif // CLEAR_MEMORY


	};


	IMG* calculate(vector<ExWithCoordinate*>* extremumList, int windowSize, string suffix, string dir) {

		int sizeDistrict = 20;
		for (int i = extremumList->size() - 1; i >= 0; i--) {
			ExWithCoordinate* exWithCoordinate = extremumList->at(i);
			if (exWithCoordinate->row - sizeDistrict < 0)
				extremumList->erase(extremumList->begin() + i);
			else if (exWithCoordinate->col - sizeDistrict < 0)
				extremumList->erase(extremumList->begin() + i);
			else if (exWithCoordinate->row + sizeDistrict >= img->height)
				extremumList->erase(extremumList->begin() + i);
			else if (exWithCoordinate->col + sizeDistrict >= img->width)
				extremumList->erase(extremumList->begin() + i);
		}

		int a = 2;
		int b = 2;
		IMG* img_gaussFilter_separable = img->gaussFilter_separable( 1.5);
		//delete img;

		IMG* img = img_gaussFilter_separable->normalize_COLOR();
#ifdef CLEAR_MEMORY
		delete img_gaussFilter_separable;
#endif // CLEAR_MEMORY





		// вычисляем градиент и производные
		{
			img_Sobol_X = img->sobelDerivativeX( IMG::edgeEffect::BLACK);
			img_normalized_Sobol_X = img_Sobol_X;

			img_Sobol_Y = img->sobelDerivativeY( IMG::edgeEffect::BLACK);
			img_normalized_Sobol_Y = img_Sobol_Y;

			img_Atan2 = IMG::atan(
				img_Sobol_X, img_Sobol_Y, img_Sobol_X->width, img_Sobol_X->height);
			img_Atan2_normalized = img_Atan2->normalize_COLOR();

			img_gradient = IMG::sobolGradient(
				img_Sobol_X, img_Sobol_Y, img_Sobol_X->width, img_Sobol_X->height);
		}

		vector<HarrisPixel*>* HarrisPixelList = new vector<HarrisPixel*>;
		for (int i = 0; i < img->getSize(); i++)
			HarrisPixelList->push_back(NULL);

		int windowHalfSize = windowSize / 2;

		vector<vector<double>*>* gaussMatrix = img->getGaussMatrix(windowSize);

		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				// вычисляем очередную матрицу в пикселе
				HarrisPixel* harrisPixel = new HarrisPixel();
				harrisPixel->isActive = false;
				HarrisPixelList->at(row * img->width + col) = harrisPixel;
			}
		}
		for (int extremum_index = 0; extremum_index < extremumList->size(); extremum_index++) {
			ExWithCoordinate* extremum = extremumList->at(extremum_index);
			int row = extremum->row;
			int col = extremum->col;

			double A = 0.0;
			double B = 0.0;
			double C = 0.0;
			for (int y = -windowHalfSize; y <= windowHalfSize; ++y) {
				for (int x = -windowHalfSize; x <= windowHalfSize; ++x) {
					int index_X = col + x;
					int index_Y = row + y;
					// один из пикселей окна


					double pixel_sobol_X_red = img_normalized_Sobol_X->getRedWithEdge(index_Y, index_X);	// TODO remove
					double pixel_sobol_X_green = img_normalized_Sobol_X->getGreenWithEdge(index_Y, index_X);// TODO remove
					double pixel_sobol_X_blue = img_normalized_Sobol_X->getBlueWithEdge(index_Y, index_X);// TODO remove
					double pixel_sobol_X_gray = img_normalized_Sobol_X->getGrayWithEdge(index_Y, index_X);

					double pixel_sobol_Y_red = img_normalized_Sobol_Y->getRedWithEdge(index_Y, index_X);// TODO remove
					double pixel_sobol_Y_green = img_normalized_Sobol_Y->getGreenWithEdge(index_Y, index_X);// TODO remove
					double pixel_sobol_Y_blue = img_normalized_Sobol_Y->getBlueWithEdge(index_Y, index_X);// TODO remove
					double pixel_sobol_Y_gray = img_normalized_Sobol_Y->getGrayWithEdge(index_Y, index_X);

					int gauss_X = x + windowHalfSize;
					int gauss_y = y + windowHalfSize;

					A += pixel_sobol_X_gray * pixel_sobol_X_gray * gaussMatrix->at(gauss_y)->at(gauss_X);
					B += pixel_sobol_X_gray * pixel_sobol_Y_gray * gaussMatrix->at(gauss_y)->at(gauss_X);
					C += pixel_sobol_Y_gray * pixel_sobol_Y_gray * gaussMatrix->at(gauss_y)->at(gauss_X);
				}
			}
			// вычисляем очередную матрицу в пикселе
			HarrisPixel* gg = new HarrisPixel(row, col, A, B, C);
			HarrisPixelList->at(row * img->width + col) = gg;
		}
		

#ifdef CLEAR_MEMORY
		delete img_Sobol_X;
		delete img_Sobol_Y;
#endif // CLEAR_MEMORY


		//----------------------------------------------------------------------------------------------------
		// Ищем точки с большим откликом L_min > Threshold
		// сортируем по L_min
		vector<HarrisPixel*>* HarrisPixelList_sortedLmin = new vector<HarrisPixel*>();

		for (int i = 0; i < HarrisPixelList->size(); i++) {
			HarrisPixelList_sortedLmin->push_back(HarrisPixelList->at(i));
			//if (i < 50)
			//	cout << HarrisPixelList->at(i)->L_min << "\n";
		}
		// TODODO
		sort(HarrisPixelList_sortedLmin->begin(), HarrisPixelList_sortedLmin->end(),
			[](const HarrisPixel* first, const HarrisPixel* second) {
				if (first == NULL && second == NULL) {
					return true;
				}
				if (first == NULL && second != NULL) {
					return true;
				}
				else if (first != NULL && second == NULL) {
					return false;
				}
				else {
					return first->L_min < second->L_min;
				}
			});
		/*for (int i = 0; i < HarrisPixelList_sortedLmin->size(); i++) {
			if (HarrisPixelList_sortedLmin->at(i)->L_min > -1) {
				cout << HarrisPixelList_sortedLmin->at(i)->L_min << endl;
			}
		}*/
		// получаем значение Threshold
		int top_x_level_index = (int)(HarrisPixelList_sortedLmin->size() * TOP_X_LEVEL);

		HarrisPixel* top_XTmp = NULL;
		do {
			top_XTmp = HarrisPixelList_sortedLmin->at(top_x_level_index);
			top_x_level_index++;
		} while (top_XTmp == NULL);
		HarrisPixel* top_X = top_XTmp;


		cout << ("====================================top_X===========================") << endl;
		cout << ("Treshhold = " + to_string(top_X->L_min)) << endl;
		cout << ("====================================top_X===========================") << endl;


		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
		top_X->L_min = LAB6_HARRIS_LEVEL;

		// "удаляем" точки где L_min < Threshold
		int countOK = 0;
		for (int i = 0; i < HarrisPixelList->size(); i++) {
			/*if (HarrisPixelList->at(i)->L_min > -100 ) {
				cout << "i = " << to_string(i) << "\t\tvalue =" <<  HarrisPixelList->at(i)->L_min << endl;
			}*/
			
			if (HarrisPixelList->at(i)->L_min < top_X->L_min)
				HarrisPixelList->at(i)->isActive = false;
			else
				countOK++;
		}
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		for (int i = 0; i < HarrisPixelList->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList->at(i);
			if (HarrisPixel->row < 10 || HarrisPixel->col < 10)
				HarrisPixel->isActive = false;
			if (img->height - 10 < HarrisPixel->row || img->width - 10 < HarrisPixel->col)
				HarrisPixel->isActive = false;
		}

		//----------------------------------------------------------------------------------------------------
		// Требование локального максимума
		// оставляем только локальные максимумы
		// тут сохраним индексы, точек, которые надо отбросить
		// потому что сразу, во время прохода нельзя менять массив
		vector<int>* indexForRemoveLocal = new vector<int>;
		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				HarrisPixel* currentHarrisPixel = HarrisPixelList->at(row * img->width + col);

				double current_L_min = currentHarrisPixel->L_min;
				// координаты окна, в котормо проверяем на локальный максимум
				int row_start = max(row - POINT_LOCAL_SIZE, 0);
				int col_start = max(col - POINT_LOCAL_SIZE, 0);
				int row_finish = min(row + POINT_LOCAL_SIZE, img->height - 1);
				int col_finish = min(col + POINT_LOCAL_SIZE, img->width - 1);
				bool flag_isCurrentLargest = true;
				for (int y = row_start; flag_isCurrentLargest && y <= row_finish; ++y) {
					for (int x = col_start; flag_isCurrentLargest && x <= col_finish; ++x) {
						// Если точка в локальной окрестности больше нашей рассматриваемой
						// или равна ей, то надо грохнуть нашу точку
						if (current_L_min < HarrisPixelList->at(y * img->width + x)->L_min ||
							abs(HarrisPixelList->at(y * img->width + x)->L_min - current_L_min) < 0.0000000001
							) {
							if (x == col && y == row)
								continue;
							flag_isCurrentLargest = false;
							indexForRemoveLocal->push_back(row * img->width + col);
						}
					}
				}
			}
		}

		// вот теперь можно все ненужные точки дропнуть
		for (int i = 0; i < (int)indexForRemoveLocal->size(); i++) {
			int integer = indexForRemoveLocal->at(i);
			HarrisPixelList->at(integer)->isActive = false;
		}
		// отклик
//		IMG* newImg = img->copy();
//		double max = -9999999999999.0;
//
//		for (int i = 0; i < ((int)HarrisPixelList_sortedLmin->size()); i++) {
//			HarrisPixel* HarrisPixel = HarrisPixelList_sortedLmin->at(i);
//
//			if (max < HarrisPixel->L_min)
//				max = HarrisPixel->L_min;
//
//
//			// TODO грохнуть лишние цвета
//			if (HarrisPixel->L_min > 0) {
//				newImg->pixels_red[newImg->width * HarrisPixel->row + HarrisPixel->col] = HarrisPixel->L_min;
//				newImg->pixels_green[newImg->width * HarrisPixel->row + HarrisPixel->col] = HarrisPixel->L_min;
//				newImg->pixels_blue[newImg->width * HarrisPixel->row + HarrisPixel->col] = HarrisPixel->L_min;
//				newImg->pixels_gray[newImg->width * HarrisPixel->row + HarrisPixel->col] = HarrisPixel->L_min;
//			}
//			else {
//			}
//		
//		}
//
//		IMG* newImg_normalize = newImg->normalize_GRAY();
//
//
//		//newImg_normalize->saveImage_GRAY("values" + suffix + ".jpg", dir);
//#ifdef CLEAR_MEMORY
//		delete newImg;
//		delete newImg_normalize;
//#endif // CLEAR_MEMORY


		//----------------------------------------------------------------------------------------------------
		// все найденные точки
		IMG* img_all = drawRedPoint(HarrisPixelList);
		IMG* img_all_normalize = img_all->normalize_COLOR();
		//img_all_normalize->saveImage_COLOR("all_" + suffix + ".jpg", dir);
#ifdef CLEAR_MEMORY
		delete img_all;
		delete img_all_normalize;
#endif // CLEAR_MEMORY

		
		//----------------------------------------------------------------------------------------------------
		// оставляем POINT_COUNT_TOP_POWERFUL сильнейших точек
		// сортируем по L_min
		/*
		vector<HarrisPixel*>* HarrisPixels_POWERFUL = topPowerful(HarrisPixelList);
		vector<pair<int, int>*>* coordinate_POWERFUL = getCoordinate(HarrisPixels_POWERFUL);
		// Рисуем красные точки на картинке
		IMG* img_powerful = drawRedPoint(HarrisPixels_POWERFUL);
		IMG* img_powerful_normalize = img_powerful->normalize_COLOR();
		//img_powerful_normalize->saveImage_COLOR("top_" + to_string(POINT_COUNT_TOP_POWERFUL) + suffix + ".jpg", dir);
		
#ifdef CLEAR_MEMORY
		delete img_powerful;
		delete img_powerful_normalize;
		for (int i = 0; i < HarrisPixels_POWERFUL->size(); i++) {
			if (HarrisPixels_POWERFUL->at(i))
				delete HarrisPixels_POWERFUL->at(i);
		}
		HarrisPixels_POWERFUL->clear();
		HarrisPixels_POWERFUL->shrink_to_fit();
#endif // CLEAR_MEMORY
		*/

		//----------------------------------------------------------------------------------------------------
		// Adaptive non-maximum suppression

		vector<HarrisPixel*>* HarrisPixels_ANMS = ANMS(HarrisPixelList);
		vector<pair<int, int>*>* coordinate_ANMS = getCoordinate(HarrisPixels_ANMS);
		// Рисуем красные точки на картинке
		IMG* img_ANMS = drawRedPoint(HarrisPixels_ANMS);
		IMG* img_ANMS_normalize = img_ANMS->normalize_COLOR();
		//img_ANMS_normalize->saveImage_COLOR("ANMS_" + to_string(POINT_COUNT_TOP_ANMS) + suffix + ".jpg", dir);

		this->indexUnicalPoint = coordinate_ANMS;

#ifdef CLEAR_MEMORY
		delete img_ANMS;
		delete img_ANMS_normalize;
		for (int i = 0; i < HarrisPixels_ANMS->size(); i++) {
			if (HarrisPixels_ANMS->at(i))
				delete HarrisPixels_ANMS->at(i);
		}
		HarrisPixels_ANMS->clear();
		HarrisPixels_ANMS->shrink_to_fit();
#endif // CLEAR_MEMORY


		//----------------------------------------------------------------------------------------------------
		// чистим остальную память

#ifdef CLEAR_MEMORY
		for (int i = 0; i < HarrisPixelList->size(); i++) {
			if (HarrisPixelList->at(i))
				delete HarrisPixelList->at(i);
		}
		HarrisPixelList->clear();
		HarrisPixelList->shrink_to_fit();

		indexForRemoveLocal->clear();
		indexForRemoveLocal->shrink_to_fit();
		delete indexForRemoveLocal;
#endif // CLEAR_MEMORY



		return this->img;
	}


	IMG* drawRedPoint(vector<HarrisPixel*>* HarrisPixelList) {
		IMG* img_copy = img->copy();
		int countPointDrawed = 0;
		for (int row = 0; row < img_copy->height; ++row) {
			for (int col = 0; col < img_copy->width; ++col) {
				HarrisPixel* currentHarrisPixel = HarrisPixelList->at(row * img_copy->width + col);
				if (!currentHarrisPixel->isActive)
					continue;

				countPointDrawed++;
				//                   if (currentHarrisPixel->col != col || currentHarrisPixel->row != row)
				//                       System.out.println();

				img_copy->setRedSquare(row, col, POINT_SIZE);
			}
		}
		cout << ("countPointDrawed = " + to_string(countPointDrawed)) << endl;
		return img_copy;
	}



	vector<HarrisPixel*>* topPowerful(vector<HarrisPixel*>* HarrisPixelList) {
		vector<HarrisPixel*>* HarrisPixelList_COPY = new vector<HarrisPixel*>();
		for (int i = 0; i < HarrisPixelList->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList->at(i);
			HarrisPixelList_COPY->push_back(HarrisPixel->copy());
		}

		vector<HarrisPixel*>* HarrisPixelList_sortedLmin2 = new vector<HarrisPixel*>();
		for (int i = 0; i < HarrisPixelList_COPY->size(); i++) {
			HarrisPixelList_sortedLmin2->push_back(HarrisPixelList_COPY->at(i));
			cout << "";
		}


		////
		vector<HarrisPixel*>* HarrisPixelList_sortedLmin3 = new vector<HarrisPixel*>();
		//filter
		for (int i = 0; i < HarrisPixelList_sortedLmin2->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList_sortedLmin2->at(i);
			if (HarrisPixel->isActive == true) {
				HarrisPixelList_sortedLmin3->push_back(HarrisPixel);
			}
		}
		//sorted
		sort(HarrisPixelList_sortedLmin3->begin(), HarrisPixelList_sortedLmin3->end(),
			[](const HarrisPixel* first, const HarrisPixel* second) {
				return first->L_min < second->L_min;
			});
		HarrisPixelList_sortedLmin2 = HarrisPixelList_sortedLmin3;

		//printFile(HarrisPixelList_sortedLmin2, "top300");
		// получаем значение
		int top_x_index = HarrisPixelList_sortedLmin2->size() - POINT_COUNT_TOP_POWERFUL;
		HarrisPixel* top_X2tmp = NULL;
		if (top_x_index < 0)
			top_x_index = 0;
		do {
			top_X2tmp = HarrisPixelList_sortedLmin2->at(top_x_index);
			top_x_index++;
		} while (top_X2tmp == NULL);
		HarrisPixel* top_X2 = top_X2tmp;


		// "удаляем" точки где L_min < Threshold
		for (int i = 0; i < HarrisPixelList_COPY->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList_COPY->at(i);
			if (HarrisPixel == NULL)
				continue;
			if (HarrisPixel->L_min < top_X2->L_min)
				HarrisPixel->isActive = false;
		}
		return HarrisPixelList_COPY;
	}

	vector<pair<int, int>*>* getCoordinate(vector<HarrisPixel*>* topPowerful) {
		vector<pair<int, int>*>* indexUnicalPoint = new vector<pair<int, int>*>;
		for (int i = 0; i < topPowerful->size(); i++) {
			HarrisPixel* HarrisPixel = topPowerful->at(i);
			if (HarrisPixel->isActive == true)
				indexUnicalPoint->push_back(
					new pair<int, int>(HarrisPixel->row, HarrisPixel->col)
				);
		}
		return indexUnicalPoint;
	}

	vector<HarrisPixel*>* ANMS(vector<HarrisPixel*>* HarrisPixelList) {

		vector<HarrisPixel*>* HarrisPixelList_COPY = new  vector<HarrisPixel*>();
		for (int i = 0; i < HarrisPixelList->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList->at(i);
			HarrisPixelList_COPY->push_back(HarrisPixel->copy());
		}


		// надо создать список только тех точек, что сейчас активны(подходят нам)
		vector<HarrisPixel*>* activePointList = new vector<HarrisPixel*>();
		for (int i = 0; i < HarrisPixelList_COPY->size(); i++) {
			HarrisPixel* HarrisPixel = HarrisPixelList_COPY->at(i);
			if (HarrisPixel->isActive == true)
				activePointList->push_back(HarrisPixel);
		}


		double r_step = 10;
		double r = r_step;
		int countNeedPoint = POINT_COUNT_TOP_ANMS;
		int currentCountPoint = 0;
		vector<int>* indexForRemoveAdaptive = new vector<int>();
		int countIteration = 0;
		do {
			currentCountPoint = 0;
			indexForRemoveAdaptive->clear();
			for (int i = 0; i < activePointList->size(); i++) {
				HarrisPixel* basePixel = activePointList->at(i);

				bool isPointOk = true;
				for (int j = 0; j < activePointList->size(); j++) {
					HarrisPixel* neightborPixel = activePointList->at(j);

					if (neightborPixel == basePixel) // сам пиксель не рассматриваем
						continue;

					double distance =
						sqrt((basePixel->row - neightborPixel->row) * (basePixel->row - neightborPixel->row) +
							(basePixel->col - neightborPixel->col) * (basePixel->col - neightborPixel->col));
					// если дистанция до пикселя больше чем надо, то тоже не рассматриваем
					if (distance > r)
						continue;

					// в окружности радиусом r нашли точку, которая больше нашей(которая в центре круга)
					if (basePixel->L_min < neightborPixel->L_min) {
						// значит точка в центре не подходит
						indexForRemoveAdaptive->push_back(basePixel->row * img->width + basePixel->col);
						isPointOk = false;
						break;
					}
				}
				if (isPointOk)
					currentCountPoint++;
			}
			if (currentCountPoint < countNeedPoint) {
				r -= r_step;
				r_step = r_step / 2;
				r += r_step;
			}
			else
				r += r_step;
			countIteration++;
			cout << "countIteration = " << countIteration << endl;
		} while (currentCountPoint > countNeedPoint || r_step > 0.1);


		// вот теперь можно все ненужные точки дропнуть
		for (int i = 0; i < indexForRemoveAdaptive->size(); i++) {
			int integer = indexForRemoveAdaptive->at(i);
			HarrisPixelList_COPY->at(integer)->isActive = false;
		}
		return HarrisPixelList_COPY;
	}

	void printToFile(vector<HarrisPixel*>* list, string filename) {
		ofstream out;          // поток для записи
		string dir = "C:/_img/result";
		out.open(dir + "/" + filename); // окрываем файл для записи
		if (out.is_open())
		{

			for (int i = 0; i < list->size(); i++) {
				out << list->at(i)->L_min << std::endl;
			}
		}

		std::cout << "End of program" << std::endl;
	}



	vector<pair<int, int>*>* getIndexUnicalPoint() {
		return this->indexUnicalPoint;
	}
};
