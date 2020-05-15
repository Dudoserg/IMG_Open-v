#pragma once
#include <string>

#include <iostream>
#include <vector>
#include "IMG.h"
using namespace std;


class Moravec
{
public:
	IMG *img;

	// полуразмер точки
	int POINT_SIZE = 2;

	// Полуразмер окна для поиска локального максмума
	int POINT_LOCAL_SIZE = 3;

	// коилчество уникальных точек
	int POINT_COUNT_MORAVEC = 100;

	vector<pair<int, int>*> *indexUnicalPoint = new vector<pair<int, int>*>;

	Moravec(IMG *img) {
		this->img = img;
	};

	IMG* calculate(int windowSize, int shift) {
		int shift_list_size_1 = 8;
		int shift_list_size_2 = 2;

		int shift_list[8][2] = {
				{shift, 0}, {shift, shift}, {0, shift}, {-shift, shift},
				{-shift, 0}, {-shift, -shift}, {0, -shift}, {shift, shift}
		};


		double sum = 0.0;
		int windowHalfSize = windowSize / 2;

		//        double cd []  = new double[img->height * img->width];
		int cd_size = img->width * img->height;
		double *cd = new double[cd_size];

		//Arrays.fill(cd, -(Double.MAX_VALUE));

		for (int i = 0; i < img->getSize(); i++) {
			cd[i] = -9999999999.0;
		}


		for (int row = windowHalfSize + shift; row < img->height - (windowHalfSize + shift); ++row) {
			for (int col = windowHalfSize + shift; col < img->width - (windowHalfSize + shift); ++col) {
				// для каждого направления считаем разницу
//                List<Double> c_arr = new ArrayList<>();
				vector<double> c_arr;

				for (int direction = 0; direction < shift_list_size_1; ++direction) {

					sum = 0.0;
					for (int y = -windowHalfSize; y <= windowHalfSize; ++y) {
						for (int x = -windowHalfSize; x <= windowHalfSize; ++x) {
							int index_X = col + x;
							int index_Y = row + y;

							//                            Pixels base = img.getImg_pixels()[index_Y * img->width + index_X];
							//Pixel *base = img->getPixels()->at(index_Y * img->width + index_X);
							double base_gray = img->pixels_gray[index_Y * img->width + index_X];

							index_X += shift_list[direction][0];
							index_Y += shift_list[direction][1];

							//                            Pixels pixel = img.getImg_pixels()[index_Y * img->width + index_X];
							//Pixel *pixel = img->getPixels()->at(index_Y * img->width + index_X);
							double pixel_gray = img->pixels_gray[index_Y * img->width + index_X];

							sum += (base_gray - pixel_gray) * (base_gray - pixel_gray);
						}
					}
					c_arr.push_back(sum);

				}
				// Значение оператора в точке
				cd[row * img->width + col] = *min_element(c_arr.begin(), c_arr.end());
			}
		}

		// тут сохраним индексы, точек, которые надо отбросить
		// потому что сразу, во время прохода нельзя менять массив
//        List<Integer> indexForRemove = new ArrayList<>();
		vector<int> indexForRemove;
		// Требование локального максимума
		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				double current = cd[row * img->width + col];
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
						if (current < cd[y * img->width + x] ||
							abs(cd[y * img->width + x] - current) < 0.00000001) {
							if (x == col && y == row)
								continue;
							flag_isCurrentLargest = false;
							indexForRemove.push_back(row * img->width + col);
						}
					}
				}
			}
		}
		// вот теперь можно все ненужные точки дропнуть
		for (int i = 0; i < indexForRemove.size(); i++) {
			int integer = indexForRemove.at(i);
			cd[integer] = -999999999;
		}

		// Копируем массив
		vector<double> cd_sorted_tmp;
		for (int i = 0; i < cd_size; i++) {
			cd_sorted_tmp.push_back(cd[i]);
		}
		sort(cd_sorted_tmp.begin(), cd_sorted_tmp.end());
		// сортируем его
		double *Cd_sorted = new double[cd_size];
		for (int i = 0; i < cd_size; i++) {
			Cd_sorted[i] = cd_sorted_tmp.at(i);
		}

		// Находим границу, после которого идет POINT_COUNT нужных точек
		double top = Cd_sorted[cd_size - POINT_COUNT_MORAVEC];
		cout << ("top = " + to_string(top));
		int countPointDrawed = 0;
		for (int row = 0; row < img->height; ++row) {
			for (int col = 0; col < img->width; ++col) {
				double c = cd[row * img->width + col];
				// Откидываем точки по порогу
				if (c > top) {
					countPointDrawed++;
					indexUnicalPoint->push_back(new pair<int, int>(row, col));

					img->setRedSquare(row, col, POINT_SIZE);

				}
			}
		}
		cout << ("countPointDrawed = " + countPointDrawed);
		return img;
	}
};
