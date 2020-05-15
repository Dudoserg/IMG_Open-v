#pragma once

#include "DEF.h"
#include "IMG.h"
#include "Descriptor.h"

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

class Descriptors
{


private:


	vector<pair<int, int>*>* listPoints = new vector<pair<int, int>*>; // особые точки
	IMG* img_Atan; // направление градиента
	IMG* img_gradient; // величина градиента

	int N = 8;        // количество корзин
	double box_size = 2 * M_PI / N;
	double box_Halfsize = M_PI / N;


	vector<Descriptor*>* descriptorList = new vector<Descriptor*>;
public:
	int sizeRegion = 4;
	int half_sizeRegion = sizeRegion / 2;
	int countRegion = 2;
	int half_countRegion = countRegion / 2;

	Descriptors(vector<pair<int, int>*>* listPoints, IMG* img_Atan, IMG* img_gradient) {
		this->listPoints = listPoints;
		this->img_Atan = img_Atan;
		this->img_gradient = img_gradient;

		this->calculateDescriptors();

		for (int i = 0; i < this->descriptorList->size(); i++) {
			this->normalizeDescriptor(this->descriptorList->at(i));
		}
	}
	~Descriptors() {
#ifdef CLEAR_MEMORY
		{
			for (int i = 0; i < listPoints->size(); i++) {
				delete listPoints->at(i);
			}
			this->listPoints->clear();
			this->listPoints->shrink_to_fit();
		}

		{
			for (int i = 0; i < descriptorList->size(); i++) {
				delete descriptorList->at(i);
			}
			this->descriptorList->clear();
			this->descriptorList->shrink_to_fit();
		}
#endif // CLEAR_MEMORY
	}

	void calculateDescriptors() {

		vector<vector<vector<double>*>*>* boxes = new vector<vector<vector<double>*>*>;
		for (int i = 0; i < sizeRegion; i++) {
			vector<vector<double>*>* line = new vector<vector<double>*>;
			for (int j = 0; j < sizeRegion; j++) {
				line->push_back(new vector<double>);
			}
			boxes->push_back(line);
		}

		for (int q = 0; q < listPoints->size(); q++) {

			int row = listPoints->at(q)->first;
			int col = listPoints->at(q)->second;


			int y = row;
			int x = col;

			// количество регионов
			for (int outY = 0; outY < countRegion; outY++) {
				for (int outX = 0; outX < countRegion; outX++) {
					int indexRegionY = outY - half_countRegion;
					int indexRegionX = outY - half_countRegion;
					// цикл внутри региона


					vector<double>* box = new vector<double>;
					for (int i = 0; i < N; i++) {
						box->push_back(0.0);
					}

					for (int inY = 0; inY < sizeRegion; inY++) {
						for (int inX = 0; inX < sizeRegion; inX++) {
							int shift_x = indexRegionX * sizeRegion;
							int shift_y = indexRegionY * sizeRegion;

							int index_X = x + inX + shift_x;
							int index_Y = y + inY + shift_y;

							addValueToBox(
								box,
								img_Atan->getGrayWithEdge(index_Y, index_X),
								img_gradient->getGrayWithEdge(index_Y, index_X)
							);

						}
					}
					boxes->at(outY)->at(outX) = box;
					cout << "";
				}
			}

			// Делаем вектор размерности 1 * XXX

			vector<double>* tmp_vector = new vector<double>;
			for (int i = 0; i < countRegion; i++) {
				for (int j = 0; j < countRegion; j++) {
					for (int n = 0; n < N; n++) {
						tmp_vector->push_back(boxes->at(i)->at(j)->at(n));
					}
				}
			}
			cout << "";
			descriptorList->push_back(new Descriptor(col, row, tmp_vector));
		}

		//vector<vector<vector<double>*>*>* boxes
		for (int i = 0; i < boxes->size(); i++) {
			//vector<vector<double>*>*
			for (int j = 0; j < boxes->at(i)->size(); j++) {
				//vector<double>*
				boxes->at(i)->at(j)->clear();
				boxes->at(i)->at(j)->shrink_to_fit();
			}
			boxes->at(i)->clear();
			boxes->at(i)->shrink_to_fit();
		}
	}

	void addValueToBox(vector<double>* box, double pixel_direction_gray, double pixel_gradient_gray){

		double rad = pixel_direction_gray;

		//если угол отрицательный, переводим его в положительную окружность
		if (rad < 0)
			rad += 2 * M_PI;

		// определяем, в какую коробку будем класть
		int num_half = (int)(rad / box_Halfsize);
		// индекс первой корзины
		int first_box_num = num_half / 2;
		// индекс второй корзины
		int second_box_num = -1337;

		int t = num_half - first_box_num * 2;
		if (t == 1) {
			// вторая часть корзины, значит вторая корзина это следующая за этой
			second_box_num = first_box_num + 1;
		}
		else if (t == 0) {
			// первая часть корзины, значит вторая корзина до этой
			second_box_num = first_box_num - 1;
		}

		// до центра основной корзины
		double first_box_distance = abs((first_box_num * box_size + box_Halfsize) - rad);
		// до центра второй корзины
		double second_box_distance = abs((second_box_num * box_size + box_Halfsize) - rad);

		// Если номер корзины выходит за границы

		if (second_box_num == N)
			second_box_num = 0;
		else if (second_box_num == -1)
			second_box_num = N - 1;

		// чем ближе к центру корзины, тем больше в корзину попадает
		double first_value_part = (second_box_distance / box_size);
		double second_value_part = (first_box_distance / box_size);

		// величина градиента

		// величина которую кладем в корзины
		double first_value = first_value_part * pixel_gradient_gray;
		double second_value = second_value_part * pixel_gradient_gray;


		// добавляем значения в корзины
		box->at(first_box_num) += first_value;
		box->at(second_box_num) += second_value;

		cout << "";
	}

	void normalizeDescriptor(Descriptor* descriptor) {
		double sum = 0.0;
		for (int i = 0; i < descriptor->vector->size(); i++) {
			sum += abs(descriptor->vector->at(i));
		}

		for (int i = 0; i < descriptor->vector->size(); i++) {
			if (descriptor->vector->at(i) / sum > 0.2) {
				descriptor->vector->at(i) = 0.2;
			}
			else {
				descriptor->vector->at(i) = descriptor->vector->at(i) / sum;
			}
		}
		//повторная нормализация,
		//позволяет снизить влияние бликов и подобных паразитных эффектов
		sum = 0.0;
		for (int i = 0; i < descriptor->vector->size(); i++) {
			sum += abs(descriptor->vector->at(i));
		}

		for (int i = 0; i < descriptor->vector->size(); i++) {
			if (descriptor->vector->at(i) / sum > 0.2) {
				descriptor->vector->at(i) = 0.2;
			}
			else {
				descriptor->vector->at(i) = descriptor->vector->at(i) / sum;
			}
		}
	}



	static IMG* createDemoImg(IMG* first, IMG* second) {
		// совмещаем изображение в одно изображение
		int firstW = first->width;
		int firstH = first->height;

		int secondW = second->width;
		int secondH = second->height;

		int resultW = firstW + secondW;
		int resultH = max(firstH, secondH);


		double* result_red = new double[resultH * resultW];
		double* result_green = new double[resultH * resultW];
		double* result_blue = new double[resultH * resultW];
		double* result_gray = new double[resultH * resultW];

		for (int row = 0; row < resultH; row++) {
			for (int col = 0; col < resultW; col++) {
				result_red[row * resultW + col] = 0.0;
				result_green[row * resultW + col] = 0.0;
				result_blue[row * resultW + col] = 0.0;
				result_gray[row * resultW + col] = 0.0;
			}
		}
		// помещаем на итоговое изображение первую картинку
		for (int row = 0; row < firstH; row++) {
			for (int col = 0; col < firstW; col++) {
				result_red[row * resultW + col] = first->pixels_red[row * firstW + col];
				result_green[row * resultW + col] = first->pixels_green[row * firstW + col];
				result_blue[row * resultW + col] = first->pixels_blue[row * firstW + col];
				result_gray[row * resultW + col] = first->pixels_gray[row * firstW + col];
			}
		}

		// помещаем на итоговое изображение вторую картинку
		for (int row = 0; row < secondH; row++) {
			for (int col = 0; col < secondW; col++) {
				// добавляем отступ слева, равный ширине первого изображения firstW
				result_red[row * resultW + col + firstW] = second->pixels_red[row * secondW + col];
				result_green[row * resultW + col + firstW] = second->pixels_green[row * secondW + col];
				result_blue[row * resultW + col + firstW] = second->pixels_blue[row * secondW + col];
				result_gray[row * resultW + col + firstW] = second->pixels_gray[row * secondW + col];
			}
		}
		return new IMG(resultW, resultH, result_red, result_green, result_blue, result_gray);
	}


	static vector<pair<Descriptor*, Descriptor*>*>* createPairs(Descriptors* first, Descriptors* second) {
		vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* pairsList =
			new vector<pair<pair<Descriptor*, Descriptor*>*, double>*>;

		for (int i = 0; i < first->descriptorList->size(); i++) {

			Descriptor* firstDescriptor = first->descriptorList->at(i);
			Descriptor* secondDescriptor = NULL;
			int indexSecond = -1;
			double distance = 9999999.0;

			vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* distanceList =
				new vector<pair<pair<Descriptor*, Descriptor*>*, double>*>;

			// Ищем ближайщий дескриптор для текущего
			for (int j = 0; j < second->descriptorList->size(); j++) {
				Descriptor* tmpSecondDescriptor = second->descriptorList->at(j);


				double tmpDistance = 0.0;

				for (int x = 0; x < firstDescriptor->vector->size(); x++) {
					tmpDistance += (firstDescriptor->vector->at(x) - tmpSecondDescriptor->vector->at(x)) *
						(firstDescriptor->vector->at(x) - tmpSecondDescriptor->vector->at(x));
				}
				tmpDistance = sqrt(tmpDistance);

				//                if (tmpDistance < distance) {
				//                    distance = tmpDistance;
				//                    indexSecond = j;
				//                    secondDescriptor = tmpSecondDescriptor;
				//                }

								// кладем текущую дистанцию в список
				//                if(distanceList.size() == 0){
				distanceList->push_back(
					new pair<pair<Descriptor*, Descriptor*>*, double>
					(new pair<Descriptor*, Descriptor*>
						(firstDescriptor, tmpSecondDescriptor), tmpDistance)
				);
				//                }
				//                for (int x = 0; x < distanceList.size(); x++) {
				//                    final Pair<Pair<Descriptor, Descriptor>, Double> pairDoublePair = distanceList.get(x);
				//                    if (tmpDistance > pairDoublePair.getValue()){
				//                        distanceList.add(x + 1, new Pair<>(new Pair<>(firstDescriptor, secondDescriptor), tmpDistance));
				//                        break;
				//                    }
				//                }
			}

			//vector<pair<pair<Descriptor*, Descriptor*>*, double>*>*
			sort(distanceList->begin(), distanceList->end(),
				[](const pair<pair<Descriptor*, Descriptor*>*, double>* first,
					const pair<pair<Descriptor*, Descriptor*>*, double>* second) {
						return first->second < second->second;
				});

			distance = distanceList->at(0)->second;
			firstDescriptor = distanceList->at(0)->first->first;
			secondDescriptor = distanceList->at(0)->first->second;
			// многозначность
			double nndr = distanceList->at(0)->second / distanceList->at(1)->second;
			if (nndr < 0.8) {
				pairsList->push_back(new pair<pair<Descriptor*, Descriptor*>*, double>
					(new pair<Descriptor*, Descriptor*>(firstDescriptor, secondDescriptor), distance));
			}
		}


		//vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* pairsList 
		sort(pairsList->begin(), pairsList->end(),
			[](const pair<pair<Descriptor*, Descriptor*>*, double>* first,
				const pair<pair<Descriptor*, Descriptor*>*, double>* second) {
					return first->second < second->second;
			});


		vector<pair<Descriptor*, Descriptor*>*>* result = new vector<pair<Descriptor*, Descriptor*>*>;

		// убираем лишнии соответствия
		int indexOk = 0;
		double level = 0.04;
		for (int i = 2; i < pairsList->size(); i++) {
			double div = pairsList->at(i)->second / pairsList->at(i - 1)->second;
			if (pairsList->at(i)->second > level)
				break;
			indexOk = i;

		}

		// дебаг
		for (int i = 0; i < pairsList->size(); i++) {
			if (i <= indexOk) {
				result->push_back(pairsList->at(i)->first);
				cout << (to_string(pairsList->at(i)->second) + "  -  add") << endl;
			}
			else {
				cout << (pairsList->at(i)->second) << endl;
			}
		}
		cout << ("\nresult.size() = " + to_string(result->size()) + "\n\n");

		return result;
	}


	static IMG* drawLine(IMG* firstImg, IMG* secondImg, IMG* resultImg, vector<pair<Descriptor*, Descriptor*>*>* pairs) {
		
		////////////////////////////////////////////////////////////////////////////////////////////
		IMG* img_forDraw = resultImg->copy();
		// Получаем объект - изображение
		Mat tmp_image = img_forDraw->createImage_COLOR();

		for (int i = 0; i < pairs->size(); i++) {
			pair<Descriptor*, Descriptor*>* current = pairs->at(i);
			Descriptor* first = current->first;
			Descriptor* second = current->second;

			// Рисуем на нем линию
			line(tmp_image, Point2f(first->x, first->y), Point2f(second->x + firstImg->width, second->y), Scalar(255, 255, 255));
		}

		// Помещаем изображение в массив пикселей
		img_forDraw->setMatToPixelsArray(tmp_image);


		return img_forDraw;
	}
};


