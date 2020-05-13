#pragma once
#include "IMG.h"
#include "HarrisPixel.h"
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;


class Harris
{
public:
	IMG *img;

	// ���������� �����
	int POINT_SIZE = 2;

	// ���������� ���� ��� ������ ���������� ��������
	int POINT_LOCAL_SIZE = 3;

	// ���������� ���������� �����

	int POINT_COUNT_HARRIS = 100;

	int POINT_COUNT_TOP_POWERFUL = 200;
	int POINT_COUNT_TOP_ANMS = 200;

	vector<pair<int, int>*>* indexUnicalPoint = new vector<pair<int, int>*>;

	///////////////////////
	IMG* img_Sobol_X;
	IMG* img_normalized_Sobol_X;

	IMG* img_Sobol_Y;
	IMG* img_normalized_Sobol_Y;

	Harris(IMG *img) {
		this->img = img;
	};


	IMG* calculate(int windowSize, string suffix, string dir) {

		img = img->gaussFilter_separable(1.5);
		img = img->normalize_COLOR();

		// ��������� �������� � �����������
		{
			img_Sobol_X = img->sobelDerivativeX(IMG::edgeEffect::BLACK);
			img_normalized_Sobol_X = img_Sobol_X;

			img_Sobol_Y = img->sobelDerivativeY(IMG::edgeEffect::BLACK);
			img_normalized_Sobol_Y = img_Sobol_Y;
		}

		vector<HarrisPixel*>* harrisPixelList = new vector<HarrisPixel*>;
		for (int i = 0; i < img->getSize(); i++)
			harrisPixelList->push_back(NULL);

		int windowHalfSize = windowSize / 2;

		vector<vector<double>*>* gaussMatrix = img->getGaussMatrix(windowSize);


		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				double A = 0.0;
				double B = 0.0;
				double C = 0.0;
				for (int y = -windowHalfSize; y <= windowHalfSize; ++y) {
					for (int x = -windowHalfSize; x <= windowHalfSize; ++x) {
						int index_X = col + x;
						int index_Y = row + y;
						// ���� �� �������� ����
						Pixel* pixel_sobol_X =
							img_normalized_Sobol_X->getPixelWithEdge(index_Y, index_X);
						Pixel* pixel_sobol_Y =
							img_normalized_Sobol_Y->getPixelWithEdge(index_Y, index_X);
						int gauss_X = x + windowHalfSize;
						int gauss_y = y + windowHalfSize;
						A += pixel_sobol_X->gray * pixel_sobol_X->gray * gaussMatrix->at(gauss_y)->at(gauss_X);
						B += pixel_sobol_X->gray * pixel_sobol_Y->gray * gaussMatrix->at(gauss_y)->at(gauss_X);
						C += pixel_sobol_Y->gray * pixel_sobol_Y->gray * gaussMatrix->at(gauss_y)->at(gauss_X);
					}
				}
				// ��������� ��������� ������� � �������
				HarrisPixel* harrisPixel = new HarrisPixel(row, col, A, B, C);
				harrisPixelList->at(row * img->width + col) = harrisPixel;
			}
		}

		//----------------------------------------------------------------------------------------------------
		// ���� ����� � ������� �������� L_min > Threshold
		// ��������� �� L_min
		vector<HarrisPixel*>* harrisPixelList_sortedLmin = new vector<HarrisPixel*>();

		for (int i = 0; i < harrisPixelList->size(); i++) {
			harrisPixelList_sortedLmin->push_back(harrisPixelList->at(i));
			//if (i < 50)
			//	cout << harrisPixelList->at(i)->L_min << "\n";
		}

		sort(harrisPixelList_sortedLmin->begin(), harrisPixelList_sortedLmin->end(),
			[](const HarrisPixel* first, const HarrisPixel* second) {
			return first->L_min < second->L_min;
		});

		// �������� ��������
		HarrisPixel* top_X = harrisPixelList_sortedLmin->at((int)(harrisPixelList_sortedLmin->size() * 0.9));
		//this.printFile(harrisPixelList_sortedLmin, "0.85_");
		cout << ("====================================top_X===========================") << endl;
		cout << ("Treshhold = " + to_string(top_X->L_min)) << endl ;
		cout << ("====================================top_X===========================") << endl;

		// "�������" ����� ��� L_min < Threshold
		int countOK = 0;
		for (int i = 0; i < harrisPixelList->size(); i++) {
			if (harrisPixelList->at(i)->L_min < top_X->L_min)
				harrisPixelList->at(i)->isActive = false;
			else
				countOK++;
		}


		//----------------------------------------------------------------------------------------------------
		// ���������� ���������� ���������
		// ��������� ������ ��������� ���������
		// ��� �������� �������, �����, ������� ���� ���������
		// ������ ��� �����, �� ����� ������� ������ ������ ������
		vector<int>* indexForRemoveLocal = new vector<int>;
		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				HarrisPixel* currentHarrisPixel = harrisPixelList->at(row * img->width + col);

				double current_L_min = currentHarrisPixel->L_min;
				// ���������� ����, � ������� ��������� �� ��������� ��������
				int row_start = max(row - POINT_LOCAL_SIZE, 0);
				int col_start = max(col - POINT_LOCAL_SIZE, 0);
				int row_finish = min(row + POINT_LOCAL_SIZE, img->height - 1);
				int col_finish = min(col + POINT_LOCAL_SIZE, img->width - 1);
				bool flag_isCurrentLargest = true;
				for (int y = row_start; flag_isCurrentLargest && y <= row_finish; ++y) {
					for (int x = col_start; flag_isCurrentLargest && x <= col_finish; ++x) {
						// ���� ����� � ��������� ����������� ������ ����� ���������������
						// ��� ����� ��, �� ���� �������� ���� �����
						if (current_L_min < harrisPixelList->at(y * img->width + x)->L_min ||
							abs(harrisPixelList->at(y * img->width + x)->L_min - current_L_min) < 0.0000000001
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

		// ��� ������ ����� ��� �������� ����� ��������
		for (int i = 0; i < (int)indexForRemoveLocal->size(); i++) {
			int integer = indexForRemoveLocal->at(i);
			harrisPixelList->at(integer)->isActive = false;
		}

		IMG* newImg = img->copy();
		double max = -9999999999999.0;

		for (int i = 0; i < ((int)harrisPixelList_sortedLmin->size()); i++) {
			HarrisPixel* harrisPixel = harrisPixelList_sortedLmin->at(i);

			if (max < harrisPixel->L_min)
				max = harrisPixel->L_min;
			newImg->getPixels()->at(newImg->width * harrisPixel->row + harrisPixel->col)->red = harrisPixel->L_min;
			newImg->getPixels()->at(newImg->width * harrisPixel->row + harrisPixel->col)->green = harrisPixel->L_min;
			newImg->getPixels()->at(newImg->width * harrisPixel->row + harrisPixel->col)->blue = harrisPixel->L_min;
			newImg->getPixels()->at(newImg->width * harrisPixel->row + harrisPixel->col)->gray = harrisPixel->L_min;
		}

		newImg = newImg->normalize_GRAY();


		newImg->saveImage_GRAY("values" + suffix + ".jpg", dir);

		{
			IMG* img_all = drawRedPoint(harrisPixelList);
			img_all = img_all->normalize_COLOR();
			img_all->saveImage_COLOR("all_" + suffix + ".jpg", dir);
		}
		//----------------------------------------------------------------------------------------------------
		// ��������� POINT_COUNT_TOP_POWERFUL ���������� �����
		// ��������� �� L_min
		{
			vector<HarrisPixel*>* harrisPixels_POWERFUL = topPowerful(harrisPixelList);
			vector<pair<int, int>*>* coordinate_POWERFUL = getCoordinate(harrisPixels_POWERFUL);
			// ������ ������� ����� �� ��������
			IMG* img_powerful = drawRedPoint(harrisPixels_POWERFUL);
			img_powerful = img_powerful->normalize_COLOR();
			img_powerful->saveImage_COLOR("top_" + to_string(POINT_COUNT_TOP_POWERFUL) + suffix + ".jpg", dir );
		}

		//----------------------------------------------------------------------------------------------------
		// Adaptive non-maximum suppression
		{
			vector<HarrisPixel*>* harrisPixels_ANMS = ANMS(harrisPixelList);
			vector<pair<int, int>*>* coordinate_ANMS = getCoordinate(harrisPixels_ANMS);
			// ������ ������� ����� �� ��������
			IMG* img_ANMS = drawRedPoint(harrisPixels_ANMS);
			img_ANMS = img_ANMS->normalize_COLOR();
			img_ANMS->saveImage_COLOR("ANMS_" + to_string(POINT_COUNT_TOP_ANMS) + suffix + ".jpg", dir);

			//           printCoordinate(coordinate_ANMS.stream()
			//                   .sorted((o1, o2) -> o1.getKey().compareTo(o2.getKey()))
			//                   .collect(Collectors.toList()), "coord_anms_x");

			//           printCoordinate(coordinate_ANMS.stream()
			//                   .sorted((o1, o2) -> o1.getValue().compareTo(o2.getValue()))
			//                   .collect(Collectors.toList()), "coord_anms_y");
		}
		return this->img;
	}


	IMG* drawRedPoint(vector<HarrisPixel*>* harrisPixelList) {
		IMG* img_copy = img->copy();
		int countPointDrawed = 0;
		for (int row = 0; row < img_copy->height; ++row) {
			for (int col = 0; col < img_copy->width; ++col) {
				HarrisPixel* currentHarrisPixel = harrisPixelList->at(row * img_copy->width + col);
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



	vector<HarrisPixel*>* topPowerful(vector<HarrisPixel*>* harrisPixelList) {
		vector<HarrisPixel*>* harrisPixelList_COPY = new vector<HarrisPixel*>();
		for (int i = 0; i < harrisPixelList->size(); i++) {
			HarrisPixel* harrisPixel = harrisPixelList->at(i);
			harrisPixelList_COPY->push_back(harrisPixel->copy());
		}

		vector<HarrisPixel*>* harrisPixelList_sortedLmin2 = new vector<HarrisPixel*>();
		for (int i = 0; i < harrisPixelList_COPY->size(); i++) {
			harrisPixelList_sortedLmin2->push_back(harrisPixelList_COPY->at(i));
			cout << "";
		}


		////
		vector<HarrisPixel*>* harrisPixelList_sortedLmin3 = new vector<HarrisPixel*>();
		//filter
		for (int i = 0; i < harrisPixelList_sortedLmin2->size(); i++) {
			HarrisPixel * harrisPixel = harrisPixelList_sortedLmin2->at(i);
			if (harrisPixel->isActive == true) {
				harrisPixelList_sortedLmin3->push_back(harrisPixel);
			}
		}
		//sorted
		sort(harrisPixelList_sortedLmin3->begin(), harrisPixelList_sortedLmin3->end(),
			[](const HarrisPixel* first, const HarrisPixel* second) {
			return first->L_min < second->L_min;
		});
		harrisPixelList_sortedLmin2 = harrisPixelList_sortedLmin3;

		//printFile(harrisPixelList_sortedLmin2, "top300");
		// �������� ��������
		HarrisPixel* top_X2 = harrisPixelList_sortedLmin2->at(
			harrisPixelList_sortedLmin2->size() - POINT_COUNT_TOP_POWERFUL);


		// "�������" ����� ��� L_min < Threshold
		for (int i = 0; i < harrisPixelList_COPY->size(); i++) {
			HarrisPixel* harrisPixel = harrisPixelList_COPY->at(i);
			if (harrisPixel == NULL)
				continue;
			if (harrisPixel->L_min < top_X2->L_min)
				harrisPixel->isActive = false;
		}
		return harrisPixelList_COPY;
	}

	vector<pair<int, int>*>* getCoordinate(vector<HarrisPixel*>* topPowerful) {
		vector<pair<int, int>*>* indexUnicalPoint = new vector<pair<int, int>*>;
		for (int i = 0; i < topPowerful->size(); i++) {
			HarrisPixel* harrisPixel = topPowerful->at(i);
			if (harrisPixel->isActive == true)
				indexUnicalPoint->push_back(
					new pair<int, int>(harrisPixel->row, harrisPixel->col)
				);
		}
		return indexUnicalPoint;
	}

	vector<HarrisPixel*>* ANMS(vector<HarrisPixel*>* harrisPixelList) {
		vector<HarrisPixel*>* harrisPixelList_COPY = new  vector<HarrisPixel*>();
		for (int i = 0; i < harrisPixelList->size(); i++) {
			HarrisPixel* harrisPixel = harrisPixelList->at(i);
			harrisPixelList_COPY->push_back(harrisPixel);
		}


		// ���� ������� ������ ������ ��� �����, ��� ������ �������(�������� ���)
		vector<HarrisPixel*>* activePointList = new vector<HarrisPixel*>();
		for (int i = 0; i < harrisPixelList_COPY->size(); i++) {
			HarrisPixel* harrisPixel = harrisPixelList_COPY->at(i);
			if (harrisPixel->isActive == true)
				activePointList->push_back(harrisPixel);
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

					if (neightborPixel == basePixel) // ��� ������� �� �������������
						continue;

					double distance =
						sqrt((basePixel->row - neightborPixel->row) * (basePixel->row - neightborPixel->row) +
						(basePixel->col - neightborPixel->col) * (basePixel->col - neightborPixel->col));
					// ���� ��������� �� ������� ������ ��� ����, �� ���� �� �������������
					if (distance > r)
						continue;

					// � ���������� �������� r ����� �����, ������� ������ �����(������� � ������ �����)
					if (basePixel->L_min < neightborPixel->L_min) {
						// ������ ����� � ������ �� ��������
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


		// ��� ������ ����� ��� �������� ����� ��������
		for (int i = 0; i < indexForRemoveAdaptive->size(); i++) {
			int integer = indexForRemoveAdaptive->at(i);
			harrisPixelList_COPY->at(integer)->isActive = false;
		}
		return harrisPixelList_COPY;
	}

	void printToFile(vector<HarrisPixel*>* list, string filename) {
		ofstream out;          // ����� ��� ������
		string dir = "C:/_img/result";
		out.open(dir + "/" + filename); // �������� ���� ��� ������
		if (out.is_open())
		{
			
			for (int i = 0; i < list->size(); i++) {
				out << list->at(i)->L_min << std::endl;
			}
		}

		std::cout << "End of program" << std::endl;
	}
};
