#pragma once
#include "DEF.h"
#include "IMG.h"
#include "Descriptor.h"

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "InterestingPoint.h"

using namespace std;

class Descriptors_turn
{
public:
	// уровень по которому отсекаю лишние соответствия
	 // они появляются например, когда интересная точка есть на первом изображении в том участе картинки,
	 // которого вообще нет на второй картинке
	 // но хоть какое то соответствие все равно будут строиться ( хоть и заведомо лишние)
	static double LEVEL;
	static bool IS_COLOR_LINE ;
	//    static double LEVEL =  0.2;
	//    static double LEVEL =  0.003;

		// список интересных точек
		// y // x
	vector<pair<int, int>*>* listPoints;
	// список интересных точек - в виде объектов
	vector<InterestingPoint*>* interestingPointList = new vector<InterestingPoint*>;

	IMG* img_Atan; // направление градиента
	IMG* img_gradient; // величина градиента

	int N = 8;        // количество корзин
	double box_size = 2 * M_PI / N;
	double box_Halfsize = M_PI / N;

	// Регионом я называю область, где ищется гистограмма
	int sizeRegion = 4;
	int half_sizeRegion = sizeRegion / 2;
	int countRegion = 4;    // количество таких регионов по вертикали и горизонтали
	int half_countRegion = countRegion / 2;

	int kek = 0;

	/**
	 * @param listPoints   Список интересных точек Pair( y , x )
	 * @param img_Atan     направление градиента
	 * @param img_gradient величина градиента
	 * @param kek          фигня для тестов
	 */
	Descriptors_turn(vector<pair<int, int>*>* listPoints, IMG* img_Atan, IMG* img_gradient, int kek) {
		this->kek = kek;
		this->listPoints = listPoints;
		this->img_Atan = img_Atan;
		this->img_gradient = img_gradient;

		// переводим неудобные пары List<Pair<int, int>>
		// - в удобные объекты интересных точек  List<InterestingPoint>
		this->createInterestingPoints();

		// ищем направление каждой интересной точки
		this->findAngleForInterestingPoints();

		// считаем дескрипторы
//        this->calculateDescriptors_firstTry();
		//this->calculateDescriptors_secondTry();    // чета я тут перемудрил
		this->calculateDescriptors_thirdTry();

		// нормализуем дескрипторы
		for (int i = 0; i < this->interestingPointList->size(); i++) {
			this->normalizeDescriptor(this->interestingPointList->at(i)->descriptor);
		}
	}

	/**
	 * ищем направление каждой интересной точки
	 */
	void findAngleForInterestingPoints() {
		// Массив в который будем создавать интересную точку
		// с такими же координатами но другим направлением ( второй пик )
		vector<InterestingPoint*>* additionalInterestingPointList = new vector<InterestingPoint*>;

		// Количество коробок
		int local_COUNT_BOX = 36;

		// итерация по интересным точкам
		for (int pointCounter = 0; pointCounter < interestingPointList->size(); pointCounter++) {
			// текущая, рассматриваемая, интересная точка
			InterestingPoint* interestingPoint = interestingPointList->at(pointCounter);
			// Получаем ее координаты
			int point_y = interestingPoint->row;
			int point_x = interestingPoint->col;
			// размер сетки 4 * 4 = 16
			int size = sizeRegion * countRegion;
			// полуразмер сетки
			int halfSize = size / 2;

			// размер одной коробки в радианах
			double local_BOX_SIZE = 2 * M_PI / local_COUNT_BOX;

			// создаем массив коробок
			vector<double>* local_box = new vector<double>;

			for (int i = 0; i < local_COUNT_BOX; i++) {
				local_box->push_back(0.0);
			}

			// Получаем ядро гаусса, под размер сетки
			vector<vector<double>*>* coreGauss = IMG::static_getGaussMatrix(size);

			// Начинаем обход сетки от [-8 до 8) по вертикали
			countALL = 0;
			countFailInCircle = 0;
			countInCircle = 0;
			for (int shift_y = -halfSize; shift_y < halfSize; shift_y++) {
				// по горизонтали
				for (int shift_x = -halfSize; shift_x < halfSize; shift_x++) {
					countALL++;
					// расчитываем координаты текущего рассматриваемого пикселя
					// (соседнего с интересной точкой пикселя в сетке)
					int y = point_y + shift_y + kek;
					int x = point_x + shift_x + kek;

					// если расстояние до него больше радиуса окружности, то игнорируем этот пиксель
					if (calculateDistance(shift_x, shift_y, 0, 0) < halfSize) {
						countFailInCircle++;
						continue;
					}

					// Получаем направление градиента в соседнем пикселе
					// Из картинки содержащий направления градиента получаем значение для текущего пикселя
					double _rad = img_Atan->getGrayWithEdge(y, x);

					// Получаем величину градиента в соседнем пикселе
					double _gradient_value = img_gradient->getGrayWithEdge(y, x);

					/////////////////////////////////////

					// Получаем номер коробки, и величину которую туда кладем
					// метод вернет две коробки
					vector<pair<int, double>*>* boxNumValue =
						getBoxNumValue(local_COUNT_BOX, local_BOX_SIZE, _rad, _gradient_value);

					// номер первой коробки
					int firstBoxNum = boxNumValue->at(0)->first;
					// значение которое добавляем в коробку по этому номеру
					double firstBoxValue = boxNumValue->at(0)->second;

					// аналогично со второй коробкой
					int secondBoxNum = boxNumValue->at(1)->first;
					double secondBoxValue = boxNumValue->at(1)->second;

					// получаем коэффициент ядра гаусса
					double core = coreGauss->at(shift_y + halfSize)->at(shift_x + halfSize);

					// добавляем в коробку по индексу соответствующее значение умноженное на ядро гаусса
					local_box->at(firstBoxNum) = local_box->at(firstBoxNum) + firstBoxValue * core;
					local_box->at(secondBoxNum) = local_box->at(secondBoxNum) + secondBoxValue * core;
					countInCircle++;
				}
			}
			// Ищем пиковое значение гистограммы
			// index, list<boxes>
			// создаем временный список - копия коробок
			vector<pair<int, double>*>* tmpList = new vector<pair<int, double>*>;
			for (int i = 0; i < local_box->size(); i++) {
				tmpList->push_back(new pair<int, double>(i, local_box->at(i)));
			}
			// сортируем по УБЫВАНИЮ значения гистограммы
			//tmpList.sort((first, second) -> - first->second.compareTo(second->second));
			// TODO
			sort(tmpList->begin(), tmpList->end(),
				[](const pair<int, double>* first, const pair<int, double>* second) {
						return first->second > second->second;
				});



			// Получаем первый пик
			pair<int, double>* firstPic = getFirstPic(tmpList);
			// получаем индекс коробки соответствующий первому пику
			int firstPic_boxNum = firstPic->first;
			// Сохраняем градус в радианах ( !!!!!!!!! я тут беру середину коробки  !!!!!!!! )
			interestingPoint->angle_rad = firstPic_boxNum * local_BOX_SIZE + (local_BOX_SIZE / 2);
			// так же сохраним номер коробки ( мб пригодится )
			interestingPoint->angle_boxNum = firstPic_boxNum;

			// Ищем второй пик ( я просто беру второе максимальное значение )
//             Pair<int, double> secondPic = tmpList.get(1);
//            int secondPic_boxNum = secondPic->first;
			pair<int, double>* secondPic = this->getSecondPic(tmpList, local_box);
			if (secondPic == NULL)
				continue;
			int secondPic_boxNum = secondPic->first;
			// Если второй пик больше 0.8 значения первого пика, то создаем новую интересную точку
			if (secondPic->second / firstPic->second > 0.8) {
				// Создаем еще одну точку интереса ( у нее будут такие же координаты как и у рассматриваемой)
				InterestingPoint* additional_interestingPoint =
					new InterestingPoint(interestingPoint->row, interestingPoint->col);
				// НО! другое направление
				additional_interestingPoint->angle_rad = secondPic_boxNum * local_BOX_SIZE + (local_BOX_SIZE / 2);
				additional_interestingPoint->angle_boxNum = secondPic_boxNum;
				additionalInterestingPointList->push_back(additional_interestingPoint);
			}
		}
		// после того как рассмотрели все исходные интересные точки,
		// добавим к ним еще и только что созданные ( со вторыми пиками гистограммы )
		for (int i = 0; i < additionalInterestingPointList->size(); i++) {
			interestingPointList->push_back(additionalInterestingPointList->at(i));
		}
	}

	pair<int, double>* getFirstPic(vector<pair<int, double>*>* sorted) {
		return sorted->at(0);
	}

	pair<int, double>* getSecondPic(vector<pair<int, double>*>* sorted, vector<double>* localBox) {
		// Наш первый пик
		pair<int, double>* firstPic = this->getFirstPic(sorted);

		int indexPicInSortedArr = 1;
		//
		do {
			pair<int, double>* secondPic = sorted->at(indexPicInSortedArr);
			int boxNum = secondPic->first;
			double value = secondPic->second;

			int indexLeft = boxNum - 1;     // индекс левого соседа второго пика в неотсортированном массиве
			int indexRight = boxNum + 1;    // индекс правого соседа
			if (indexLeft < 0)
				indexLeft = localBox->size() - 1;
			if (indexRight >= localBox->size())
				indexRight = 0;

			// Если соседи меньши, значит второй пик локальный максимум
			if (value > localBox->at(indexLeft) && value > localBox->at(indexRight)) {
				return secondPic;
			}
			else {
				indexPicInSortedArr++;
			}
		} while (indexPicInSortedArr < localBox->size() - 1);
		return NULL;
	}

	/**
	 * переводим неудобные пары - в удобные объекты интересных точек
	 */
	void createInterestingPoints() {
		for (int i = 0; i < this->listPoints->size(); i++) {
			pair<int, int>* pair = this->listPoints->at(i);
			int row = pair->first;
			int col = pair->second;
			InterestingPoint* interestingPoint = new InterestingPoint(row, col);
			interestingPointList->push_back(interestingPoint);
		}
	}

	// расчет дистанции м\у двумя точками
	double calculateDistance(int x1, int y1, int x2, int y2) {
		return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	}


	int countInCircle = 0;
	int countFailInCircle = 0;
	int countALL = 0;
	/**
	 * Расчет десктипторов интересных точек
	 */
	void calculateDescriptors_thirdTry() {
		int local_COUNT_BOX_ANGLE = 36;        // количество корзин

		// Размер корзины в радианах
		double local_BOX_SIZE = 2 * M_PI / local_COUNT_BOX_ANGLE;
		// полуразмер корзины в радианах
		double local_BOX_HALFSIZE = local_BOX_SIZE / 2;

		// размер сетки
		int size = (sizeRegion * countRegion);
		// полуразмер сетки
		int halfSize = size / 2;


		// Получаем ядро гаусса соответствующего сетке размера
		vector<vector<double>*>* coreGauss = IMG::static_getGaussMatrix(sizeRegion * countRegion);

		// Итерация по интересным точкам
		for (int q = 0; q < interestingPointList->size(); q++) {


			// тут идиотский массив трехмерный,
			// двумерный в котором еще одно измерение ( гистограмма )
			// * * * *
			// * * * *
			// * * * *
			// * * * *
			vector<vector<vector<double>*>*>* boxes = new vector<vector<vector<double>*>*>;

			// инициализация массива
			for (int i = 0; i < sizeRegion; i++) {
				vector<vector<double>*>* line = new vector<vector<double>*>;
				for (int j = 0; j < sizeRegion; j++) {
					vector<double>* gist = new vector<double>;
					for (int g = 0; g < N; g++) {
						gist->push_back(0.0);
					}
					line->push_back(gist);
				}
				boxes->push_back(line);
			}


			// рассматриваемая интересная точка
			InterestingPoint* interestingPoint = interestingPointList->at(q);
			// Получаем ее координаты
			int row = interestingPoint->row;
			int col = interestingPoint->col;
			// получаем направление данной точки в радианах
			double angle_rad = interestingPoint->angle_rad;
			cout << "angle_rad = " << angle_rad << endl;

			countInCircle = 0;
			countFailInCircle = 0;
			countALL = 0;
			for (int y = -halfSize; y < halfSize; y++) {
				for (int x = -halfSize; x < halfSize; x++) {
					//System.out.println("y = " + y + "\t\t" + "x = " + x);
					countALL++;

					int global_Y_notTurn = row + y;
					int global_X_notTurn = col + x;

					double tmp = 0.0;
					// Пересчет координат в соответствии с направлением интересной точки
					tmp = x * cos(angle_rad) + y * sin(angle_rad);
					// новое смещение по Х
					int new_x = (int)(tmp);

					// Пересчет координат в соответствии с направлением интересной точки
					tmp = y * cos(angle_rad) - x * sin(angle_rad);
					// новое смещение по Y
					int new_y = (int)(tmp);

					// если вышли за границу круга, то не принимаем во внимание данный пиксель
					if (calculateDistance(new_x, new_y, 0, 0) >= halfSize) {
						countFailInCircle++;
						continue;
					}
					countInCircle++;
					// Считаем индекс гистограммы куда будем прибавлять значение текущего пикселя
					int gistogram_index_y = (new_y + halfSize) / sizeRegion;
					int gistogram_index_x = (new_x + halfSize) / sizeRegion;

					int index_Y = row + new_y + kek;
					int index_X = col + new_x + kek;

					// получаем индекс коробки и значение которое кладем туда
					vector<pair<int, double>*>* boxNumValue = getBoxNumValue(
						N,      // количество корзин
						box_size,   // размер коробки
						// угол направление градиента в радианах
						img_Atan->getGrayWithEdge(global_Y_notTurn, global_X_notTurn) - angle_rad,
						// величина градиента
						img_gradient->getGrayWithEdge(global_Y_notTurn, global_X_notTurn)
					);
					// индекс первой коробки
					int first_box_num = boxNumValue->at(0)->first;
					// значениек которое кладем туда
					double first_value = boxNumValue->at(0)->second;

					// аналогично со второй коробкой
					int second_box_num = boxNumValue->at(1)->first;
					double second_value = boxNumValue->at(1)->second;

					// коэффициент ядра гаусса, не знаю нужен он или нет
					double core = coreGauss
						->at(new_y + (sizeRegion * countRegion) / 2)
						->at(new_x + (sizeRegion * countRegion) / 2);

					// добавлеяем в коробки текущей гистограммы
					//box.set(first_box_num, box.get(first_box_num) + first_value);
					double firstBoxOldValue = boxes
						->at(gistogram_index_y)
						->at(gistogram_index_x)
						->at(first_box_num);
					boxes
						->at(gistogram_index_y)
						->at(gistogram_index_x)
						->at(first_box_num) = firstBoxOldValue + first_value * core;
					//box.set(second_box_num, box.get(second_box_num) + second_value);
					double secondBoxOldValue = boxes
						->at(gistogram_index_y)
						->at(gistogram_index_x)
						->at(second_box_num);
					boxes
						->at(gistogram_index_y)
						->at(gistogram_index_x)
						->at(second_box_num) = secondBoxOldValue + second_value * core;
				}
			}

			// Делаем вектор размерности 1 * 128
			vector<double>* tmp_vector = new vector<double>;
			for (int i = 0; i < countRegion; i++) {
				for (int j = 0; j < countRegion; j++) {
					for (int n = 0; n < N; n++) {
						tmp_vector->push_back(boxes->at(i)->at(j)->at(n));
					}
				}
			}

			// создаем объект - дескриптор
			Descriptor* d = new Descriptor(col, row, tmp_vector);
			//descriptorList.add(d);
			interestingPoint->descriptor = d;
		}

	}

	double getDistance(int x, int y) {
		return sqrt(x * x + y * y);
	}


	/**
	 * @param local_COUNT_BOX количество корзин ( 8 или 36 или ...)
	 * @param local_BOX_SIZE  размер корзины в радианах
	 * @param rad             направление текущего пикселя
	 * @param gradient_value  величина текущего пикселя
	 * @return List<Pair < int, double>> первая, вторая корзины, Pair<int, double> номер корзины/значение
	 */
	vector<pair<int, double>*>* getBoxNumValue(int local_COUNT_BOX, double local_BOX_SIZE,
		double rad, double gradient_value) {
		//если угол отрицательный,
		// переводим его в положительную окружность от 0 до 6.28...
		do {
			if (rad < 0)
				rad += 2 * M_PI;
			if (rad > 2 * M_PI)
				rad -= 2 * M_PI;
		} while (rad < 0 || rad > 2 * M_PI);

		// рассчитываем размер коробки ( гистограммы )
		double local_BOX_HALFSIZE = local_BOX_SIZE / 2;
		// определяем, в какую коробку будем класть
		int num_half = (int)(rad / local_BOX_HALFSIZE);
		// индекс первой корзины
		int first_box_num = num_half / 2;
		// индекс второй корзины
		int second_box_num = -1337;

		int t = num_half - first_box_num * 2;
		if (t == 1) {
			// угол попал во вторую часть первой корзины
			// значит вторая корзина это следующая за первой
			second_box_num = first_box_num + 1;
		}
		else if (t == 0) {
			// угол попал в первую часть первой корзины
			// значит вторая корзина до первой
			second_box_num = first_box_num - 1;
		}

		// до центра основной корзины
		double first_box_distance =
			abs((first_box_num * local_BOX_SIZE + local_BOX_HALFSIZE) - rad);
		// до центра второй корзины
		double second_box_distance =
			abs((second_box_num * local_BOX_SIZE + local_BOX_HALFSIZE) - rad);


		// Если номер корзины выходит за границы, круга
		// т.е. если его индекс 8 ( а всего корзин 8, нумерация идет с 0)
		if (second_box_num == local_COUNT_BOX)
			second_box_num = 0; // то присваиваем еденицы
		else if (second_box_num == -1)  // если -1 то в последнюю корзину кладем
			second_box_num = local_COUNT_BOX - 1;

		// Интерполяция
		// чем ближе к центру корзины, тем больше в корзину попадает
		// расчитываем какая часть велчичины градиента попадет в первую и вторую коробки
		double first_value_part = (second_box_distance / local_BOX_SIZE);
		double second_value_part = (first_box_distance / local_BOX_SIZE);

		// величина градиента

		// величина которую кладем в корзины
		double first_value = first_value_part * gradient_value;
		double second_value = second_value_part * gradient_value;

		vector<pair<int, double>*>* result = new vector<pair<int, double>*>;
		result->push_back(new pair<int, double>(first_box_num, first_value));
		result->push_back(new pair<int, double>(second_box_num, second_value));

		return result;
	}


	/**
	 * Нормализуем дескриптор
	 *
	 * @param descriptor дескриптор
	 */
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
//        sum = 0.0;
//        for (int i = 0; i < descriptor->vector.size(); i++) {
//            sum += descriptor->vector.get(i) * descriptor->vector.get(i);
//        }
//        sum = sqrt(sum);
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


	int count = 0;
	static int count_nndrOk;

	/**
	 * Создаем пары интересных точек
	 * Сравниваем дескрипторы первого и второго изображения, и находим наилучшие соответствия
	 *
	 * @param first  Дескрипторы первого изображения
	 * @param second второго
	 * @return
	 */
	static vector<pair<InterestingPoint*, InterestingPoint*>*>* createPairs(
		Descriptors_turn* first, Descriptors_turn* second) {
		vector<pair<pair<InterestingPoint*, InterestingPoint*>*, double>*>* pairsList 
			= new vector<pair<pair<InterestingPoint*, InterestingPoint*>*, double>*>;

		for (int i = 0; i < first->interestingPointList->size(); i++) {

			// один из дескрипторов первого изоб
			InterestingPoint* firstInterestingPoint = first->interestingPointList->at(i);
			InterestingPoint* secondInterestingPoint = NULL;
			int indexSecond = -1;
			double distance = 9999999.0;

			vector<pair<pair<InterestingPoint*, InterestingPoint*>*, double>*>* distanceList 
				= new vector<pair<pair<InterestingPoint*, InterestingPoint*>*, double>*>;
			// Ищем ближайщий дескриптор во втором изображении для текущего из первого изображения
			for (int j = 0; j < second->interestingPointList->size(); j++) {
				// дескриптор интересной точки второго изображения.
				secondInterestingPoint = second->interestingPointList->at(j);
				InterestingPoint* tmpInterestingPoint = secondInterestingPoint;

				double tmpDistance = 0.0;

				// Сравнивать будем Эвклидовым расстоянием
				switch (0) {
					// Эвклидово
				case 0: {
					for (int x = 0; x < firstInterestingPoint->descriptor->vector->size(); x++) {
						tmpDistance += (firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x)) *
							(firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x));
					}
					tmpDistance = sqrt(tmpDistance);
					break;
				}
					  // Манхэттена
				case 1: {
					for (int x = 0; x < firstInterestingPoint->descriptor->vector->size(); x++) {
						tmpDistance += abs((firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x)));
					}
					break;
				}
				case 2: {
					for (int x = 0; x < firstInterestingPoint->descriptor->vector->size(); x++) {
						tmpDistance += (firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x)) *
							(firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x));
					}
					break;
				}
				default: {
					for (int x = 0; x < firstInterestingPoint->descriptor->vector->size(); x++) {
						tmpDistance += (firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x)) *
							(firstInterestingPoint->descriptor->vector->at(x) - secondInterestingPoint->descriptor->vector->at(x));
					}
					tmpDistance = sqrt(tmpDistance);
				}
				}

				// кладем текущую дистанцию в список
				distanceList->push_back(
					new pair<pair<InterestingPoint*, InterestingPoint*>*, double>(
						new pair<InterestingPoint*, InterestingPoint*>(firstInterestingPoint, tmpInterestingPoint),
						tmpDistance
						)
				);

			}
			// сортируем список по расстоянию ( по возрастанию )
			// в спеске пары (дескриптор первого изображени, дескриптор второго, расстояние между ними)
			//distanceList.sort(Comparator.comparingdouble(Pair::getValue));
			// TODO
			sort(distanceList->begin(), distanceList->end(),
				[](const pair<pair<InterestingPoint*, InterestingPoint*>*, double>* f,
					const pair<pair<InterestingPoint*, InterestingPoint*>*, double>* s) {
					return f->second < s->second;
				});


			// берем наименьшее расстояние
			distance = distanceList->at(0)->second;
			firstInterestingPoint = distanceList->at(0)->first->first;
			secondInterestingPoint = distanceList->at(0)->first->second;
			// проверка на многозначность
			double nndr = distanceList->at(0)->second / distanceList->at(1)->second;
			cout << "nndr = " << nndr ;

			if (nndr < 0.8) {
				cout << "    ++++++";
				count_nndrOk++;
				pairsList->push_back(
					new pair<pair<InterestingPoint*, InterestingPoint*>*, double>(
						new pair<InterestingPoint*, InterestingPoint*>(firstInterestingPoint, secondInterestingPoint),
						distance
						)
				);
			}
			cout << endl;
		}
		cout << endl << endl << " count_nndrOk = " << count_nndrOk << endl << endl;
		// Сортируем список соответствий интересных точек по возрастанию расстояния м\у точками
		//pairsList.sort(Comparator.comparingdouble(Pair::getValue));
		// TODO
		sort(pairsList->begin(), pairsList->end(),
			[](const pair<pair<InterestingPoint*, InterestingPoint*>*, double>* f,
				const pair<pair<InterestingPoint*, InterestingPoint*>*, double>* s) {
					return f->second < s->second;
			});


		// результат сохраним сюда
		vector<pair<InterestingPoint*, InterestingPoint*>*>* result 
			= new vector<pair<InterestingPoint*, InterestingPoint*>*>;

		// убираем лишнии соответствия
		int indexOk = 0;
		for (int i = 2; i < pairsList->size(); i++) {
			//double div = pairsList.get(i)->second / pairsList.get(i - 1)->second;
			indexOk = i;
			if (pairsList->at(i)->second > Descriptors_turn::LEVEL)
				break;
		}
		int debug_indexOfAddedPair = 0;
		// дебаг, для вычислений не имеет значения
		for (int i = 0; i < pairsList->size(); i++) {
			if (i <= indexOk) {
				result->push_back(pairsList->at(i)->first);
				InterestingPoint* firstInterestingPoint = pairsList->at(i)->first->first;
				InterestingPoint* secondInterestingPoint = pairsList->at(i)->first->second;
				cout << (
					"[" + to_string(firstInterestingPoint->descriptor->x) + " ; " +
						to_string(firstInterestingPoint->descriptor->y) + "]"
					);
				cout << (" ");
				cout << (
					"[" + to_string(secondInterestingPoint->descriptor->x) + " ; " +
						to_string(secondInterestingPoint->descriptor->y) + "]"
					);
				cout << ("    ");
				cout << (to_string(pairsList->at(i)->second) + "  -  added #" + to_string(debug_indexOfAddedPair++)) << endl;

			}
			else {
				InterestingPoint* firstInterestingPoint = pairsList->at(i)->first->first;
				InterestingPoint* secondInterestingPoint = pairsList->at(i)->first->second;
				cout << (
					"[" + to_string(firstInterestingPoint->descriptor->x) + " ; " + 
						to_string(firstInterestingPoint->descriptor->y) + "]");
				cout << (" ");
				cout << (
					"[" + to_string(secondInterestingPoint->descriptor->x) + " ; " + 
						to_string(secondInterestingPoint->descriptor->y) + "]");
				cout << ("    ");
				cout << (to_string(pairsList->at(i)->second) + "  #" + to_string(debug_indexOfAddedPair++)) << endl;

			}
		}

		cout << ("\nresult.size() = " + to_string(result->size()) + "\n\n") << endl;

		return result;
	}


	/**
	 * Склеиваем два изображения в одно( помещаем одно рядом с другим, в соответствии с их размерами)
	 * Если одна картинка больше другой, то появится черная область
	 *
	 * @param first  первое изображение
	 * @param second второе
	 * @return склеенные в одно две картинки
	 */
	static IMG* createDemoImg(IMG* first, IMG* second) {
		// совмещаем изображение в одно изображение
		int firstW = first->width;
		int firstH = first->height;

		int secondW = second->width;
		int secondH = second->height;

		int resultW = firstW + secondW;
		int resultH = max(firstH, secondH);

		//Pixels[] result = new Pixels[resultW * resultH];
		double* pixels_red = new double[resultW * resultH];
		double* pixels_green = new double[resultW * resultH];
		double* pixels_blue = new double[resultW * resultH];
		double* pixels_gray = new double[resultW * resultH];

		for (int row = 0; row < resultH; row++) {
			for (int col = 0; col < resultW; col++) {
				//result[row * resultW + col] = new Pixels(0.0, 0.0, 0.0, 0.0, 0.0);
				pixels_red[row * resultW + col] = 0.0;
				pixels_green[row * resultW + col] = 0.0;
				pixels_blue[row * resultW + col] = 0.0;
				pixels_gray[row * resultW + col] = 0.0;
			}
		}
		// помещаем на итоговое изображение первую картинку
		for (int row = 0; row < firstH; row++) {
			for (int col = 0; col < firstW; col++) {
				//result[row * resultW + col] = first.getImg_pixels()[row * firstW + col];
				pixels_red[row * resultW + col] = first->pixels_red[row * firstW + col];
				pixels_green[row * resultW + col] = first->pixels_green[row * firstW + col];
				pixels_blue[row * resultW + col] = first->pixels_blue[row * firstW + col];
				pixels_gray[row * resultW + col] = first->pixels_gray[row * firstW + col];
			}
		}

		// помещаем на итоговое изображение вторую картинку
		for (int row = 0; row < secondH; row++) {
			for (int col = 0; col < secondW; col++) {
				// добавляем отступ слева, равный ширине первого изображения firstW
				//result[row * resultW + col + firstW] = second.getImg_pixels()[row * secondW + col];
				pixels_red[row * resultW + col + firstW] = second->pixels_red[row * secondW + col];
				pixels_green[row * resultW + col + firstW] = second->pixels_green[row * secondW + col];
				pixels_blue[row * resultW + col + firstW] = second->pixels_blue[row * secondW + col];
				pixels_gray[row * resultW + col + firstW] = second->pixels_gray[row * secondW + col];
			}
		}
		return new IMG (resultW, resultH, pixels_red, pixels_green, pixels_blue, pixels_gray);
	}


	static IMG* drawLine(
		IMG* firstImg,
		IMG* secondImg,
		IMG* resultImg,
		vector<pair<InterestingPoint*, InterestingPoint*>*>* pairs
	) {

		////////////////////////////////////////////////////////////////////////////////////////////
		IMG* img_forDraw = resultImg->copy();
		// Ïîëó÷àåì îáúåêò - èçîáðàæåíèå
		Mat tmp_image = img_forDraw->createImage_COLOR();

		for (int i = 0; i < pairs->size(); i++) {
			pair<InterestingPoint*, InterestingPoint*>* current = pairs->at(i);
			InterestingPoint* first = current->first;
			InterestingPoint* second = current->second;

			if (IS_COLOR_LINE) {
				double* colors = getRandomColor();

				line(
					tmp_image,
					Point2f(first->descriptor->x, first->descriptor->y),
					Point2f(second->descriptor->x + firstImg->width, second->descriptor->y),
					Scalar(colors[0], colors[1], colors[2])
				);
				delete[] colors;
			}
			else {
				line(
					tmp_image,
					Point2f(first->descriptor->x, first->descriptor->y),
					Point2f(second->descriptor->x + firstImg->width, second->descriptor->y),
					Scalar(255, 255, 255)
				);
			}
		
		}
		img_forDraw->setMatToPixelsArray(tmp_image);


		return img_forDraw;
	}

	
	static double* getRandomColor() {
		double* result = new double[3];
		
		int rnd = randomBetween(0, 5);
		switch (rnd)
		{
		case 0: {
			result[0] = 255;
			result[1] = 0;
			result[2] = 0;
			break;
		}
		case 1: {
			result[0] = 0;
			result[1] = 255;
			result[2] = 0;
			break;
		}
		case 2: {
			result[0] = 0;
			result[1] = 0;
			result[2] = 255;
			break;
		}
		case 3: {
			result[0] = 0;
			result[1] = 255;
			result[2] = 255;
			break;
		}
		case 4: {
			result[0] = 255;
			result[1] = 0;
			result[2] = 255;
			break;
		}
		case 5: {
			result[0] = 255;
			result[1] = 255;
			result[2] = 0;
			break;
		}
		default:
			break;
		}
		
		return result;
	}

	static int randomBetween(double min, double max)
	{
		return rand() * ((max - min + 0.999) / RAND_MAX) + min;
	}
	

	/**
 * Рисуем направления интересных точек
 *
 * @param resultImg изображение, на котором мы рисуем направленияя каждой из интересных точек
 * @return изображения с линиями
 */
	IMG* drawArrows(IMG* resultImg) {
		vector<pair<pair<int, int>*, pair<int, int>*>*>* pairs = new vector<pair<pair<int, int>*, pair<int, int>*>*>;
		for (int i = 0; i < this->interestingPointList->size(); i++) {
			InterestingPoint* interestingPoint = this->interestingPointList->at(i);
			int col = interestingPoint->col;
			int row = interestingPoint->row;
			double fi = interestingPoint->angle_rad;
			//            final double fi = interestingPoint.getAngle_gradient_TEST();
			//            final double fi = img_Atan.getPixelWithEdge(row, col).getGray();

			int r = 20; // длина линии
			// получаем координаты конца линии
			int shift_x = (int)(r * cos(fi));
			int shift_y = (int)(r * sin(fi));

			int x = shift_x + col;
			int y = shift_y + row;
			// запоминаем координаты
			pairs->push_back(
				new pair<pair<int, int>*, pair<int, int>*>(
					new pair<int, int>(row, col), new pair<int, int>(y, x)
					)
			);
		}

		IMG* img_forDraw = resultImg->copy();

		Mat tmp_image = img_forDraw->createImage_COLOR();

		for (int i = 0; i < pairs->size(); i++) {
			pair<pair<int, int>*, pair<int, int>*>* pair = pairs->at(i);

			int first_y = pair->first->first;   // row
			int first_x = pair->first->second; // col

			int second_y = pair->second->first;    // row
			int second_x = pair->second->second;  // col

			line(
				tmp_image,
				Point2f(first_x, first_y),
				Point2f(second_x, second_y),
				Scalar(255, 255, 255)
			);
			
		}

		img_forDraw->setMatToPixelsArray(tmp_image);


		return img_forDraw;
	}

};

double Descriptors_turn::LEVEL = 0.05;
int Descriptors_turn::count_nndrOk = 0;
bool Descriptors_turn::IS_COLOR_LINE = true;