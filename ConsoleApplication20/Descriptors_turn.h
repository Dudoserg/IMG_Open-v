#pragma once
#include "DEF.h"
#include "IMG.h"
#include "Descriptor.h"

#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

class Descriptors_turn
{
public:
    // особые точки
    vector<pair<int, int>*>* listPoints;
    IMG* img_Atan; // направление градиента
    IMG* img_gradient; // величина градиента

    int N = 8;        // количество корзин
    double box_size = 2 * M_PI / N;
    double box_Halfsize = M_PI / N;


    int sizeRegion = 4;
    int half_sizeRegion = sizeRegion / 2;
    int countRegion = 4;
    int half_countRegion = countRegion / 2;


    vector<Descriptor*>* descriptorList = new vector<Descriptor*>;

    int kek = 0;

    Descriptors_turn(vector<pair<int, int>*>* listPoints, IMG* img_Atan, IMG* img_gradient, int kek) {
        this->kek = kek;
        this->listPoints = listPoints;
        this->img_Atan = img_Atan;
        this->img_gradient = img_gradient;

        this->calculateDescriptors();

        for (int i = 0; i < this->descriptorList->size(); i++) {
            this->normalizeDescriptor(this->descriptorList->at(i));
        }
    }

    /**
     * @param point_y координаты точки, у которой ищем направление
     * @param point_x
     * @return pair<int, double>, где int - номер корзины, double значение
     */
    pair<int, double>* calculateAngle(int point_y, int point_x, int local_COUNT_BOX) {
        int size = sizeRegion * countRegion;
        int halfSize = size / 2;
        double angle = 0.0;


        double local_BOX_SIZE = 2 * M_PI / local_COUNT_BOX;

        vector<double>* local_box = new vector<double>();
        for (int i = 0; i < local_COUNT_BOX; i++) {
            local_box->push_back(0.0);
        }

        vector<vector<double>*>* coreGauss = IMG::static_getGaussMatrix(size);

        for (int shift_y = -halfSize; shift_y < halfSize; shift_y++) {
            for (int shift_x = -halfSize; shift_x < halfSize; shift_x++) {
                int y = point_y + shift_y;
                int x = point_x + shift_x;


                double _rad = img_Atan->getGrayWithEdge(y, x);
                double _gradient_value = img_gradient->getGrayWithEdge(y, x);

                vector<pair<int, double>*>* boxNumValue =
                    getBoxNumValue(local_COUNT_BOX, local_BOX_SIZE, _rad, _gradient_value);

                int firstBoxNum = boxNumValue->at(0)->first;
                double firstBoxValue = boxNumValue->at(0)->second;

                int secondBoxNum = boxNumValue->at(1)->first;
                double secondBoxValue = boxNumValue->at(1)->second;

                double core = coreGauss->at(shift_y + halfSize)->at(shift_x + halfSize);
                local_box->at(firstBoxNum) += firstBoxValue * 1;
                local_box->at(secondBoxNum) += secondBoxValue * 1;
            }
        }
        // Ищем пиковое значение гистограммы
        int index_maxValue = 0;
        double maxValue = local_box->at(index_maxValue);

        for (int i = 1; i < local_box->size(); i++) {
            if (local_box->at(i) > maxValue) {
                maxValue = local_box->at(i);
                index_maxValue = i;
            }
        }
        return new pair<int, double>(index_maxValue, maxValue);
    }

    vector<pair<pair<int, int>*, double>*>* pointAngle = new vector<pair<pair<int, int>*, double>*>;

    void calculateDescriptors() {
        int local_COUNT_BOX_ANGLE = 36;        // количество корзин

        double local_BOX_SIZE = 2 * M_PI / local_COUNT_BOX_ANGLE;
        double local_BOX_HALFSIZE = local_BOX_SIZE / 2;

        vector<vector<vector<double>*>*>* boxes = new vector<vector<vector<double>*>*>;

        /////////////????????????????????????
        for (int i = 0; i < sizeRegion; i++) {
            vector<vector<double>*>* line = new vector<vector<double>*>;
            for (int j = 0; j < sizeRegion; j++) {
                line->push_back(new vector<double>());
            }
            boxes->push_back(line);
        }
        vector<vector<double>*>* coreGauss = IMG::static_getGaussMatrix(sizeRegion * countRegion);

        for (int q = 0; q < listPoints->size(); q++) {
            int row = listPoints->at(q)->first;
            int col = listPoints->at(q)->second;
            int y = row;
            int x = col;

            pair<int, double>* pairAngleValue = calculateAngle(y, x, local_COUNT_BOX_ANGLE);
            int angle_boxNum = pairAngleValue->first;
            double angle_boxValue = pairAngleValue->second;

            double angle_rad = (angle_boxNum * local_BOX_SIZE) + local_BOX_HALFSIZE;
            //            angle_rad = -1 * angle_rad;
                        //double angle_rad = angle * (M_PI / 180);
            int size = (sizeRegion * countRegion);
            int halfSize = size / 2;



            for (int outY = 0; outY < countRegion; outY++) {
                for (int outX = 0; outX < countRegion; outX++) {
                    int indexRegionY = outY - half_countRegion;
                    int indexRegionX = outY - half_countRegion;
                    // цикл внутри региона

                    vector<double>* box = new vector<double>();
                    for (int i = 0; i < N; i++) {
                        box->push_back(0.0);
                    }

                    for (int inY = 0; inY < sizeRegion; inY++) {
                        for (int inX = 0; inX < sizeRegion; inX++) {
                            int shift_x = indexRegionX * sizeRegion;
                            int shift_y = indexRegionY * sizeRegion;


                            double tmp = 0.0;
                            tmp = shift_x * cos(angle_rad) + shift_y * sin(angle_rad);
                            int new_shift_x = (int)round(tmp);
                            tmp = shift_y * cos(angle_rad) - shift_x * sin(angle_rad);
                            int new_shift_y = (int)round(tmp);

                            int index_X = x + inX + new_shift_x + kek;
                            int index_Y = y + inY + new_shift_y + kek;


                            vector<pair<int, double>*>* boxNumValue = getBoxNumValue(
                                N,
                                box_size,
                                img_Atan->getGrayWithEdge(index_Y, index_X) - angle_rad,          /// радианы
                                img_gradient->getGrayWithEdge(index_Y, index_X)
                            );
                            int first_box_num = boxNumValue->at(0)->first;
                            double first_value = boxNumValue->at(0)->second;

                            int second_box_num = boxNumValue->at(1)->first;
                            double second_value = boxNumValue->at(1)->second;

                            double core = coreGauss->at(shift_y + (sizeRegion * countRegion) / 2)
                                ->at(shift_x + (sizeRegion * countRegion) / 2);

                            box->at(first_box_num) += first_value;
                            box->at(second_box_num) += second_value;

                        }
                    }

                    boxes->at(outY)->at(outX) = box;
                }
            }

            // Делаем вектор размерности 1 * XXX
            vector<double>* tmp_vector = new vector<double>();
            for (int i = 0; i < countRegion; i++) {
                for (int j = 0; j < countRegion; j++) {
                    for (int n = 0; n < N; n++) {
                        tmp_vector->push_back(boxes->at(i)->at(j)->at(n));
                    }
                }
            }
            descriptorList->push_back(new Descriptor(col, row, tmp_vector));
        }

    }


    /**
     * @param local_COUNT_BOX количество корзин ( 8 или 36 или ...)
     * @param local_BOX_SIZE  размер корзины в радианах
     * @param rad             направление текущего пикселя
     * @param gradient_value  величина текущего пикселя
     * @return List<pair < int, double>> первая, вторая корзины, pair<int, double> номер корзины/значение
     */
    vector<pair<int, double>*>* getBoxNumValue(int local_COUNT_BOX, double local_BOX_SIZE,
        double rad, double gradient_value) {
        //если угол отрицательный, переводим его в положительную окружность
        do {
            if (rad < 0)
                rad += 2 * M_PI;
            if (rad > 2 * M_PI)
                rad -= 2 * M_PI;
        } while (rad < 0 || rad > 2 * M_PI);
        //        System.out.println(rad);

        double local_BOX_HALFSIZE = local_BOX_SIZE / 2;
        // определяем, в какую коробку будем класть
        int num_half = (int)(rad / local_BOX_HALFSIZE);
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
        double first_box_distance = abs((first_box_num * local_BOX_SIZE + local_BOX_HALFSIZE) - rad);
        // до центра второй корзины
        double second_box_distance = abs((second_box_num * local_BOX_SIZE + local_BOX_HALFSIZE) - rad);

        // Если номер корзины выходит за границы
        count++;

        if (second_box_num == local_COUNT_BOX)
            second_box_num = 0;
        else if (second_box_num == -1)
            second_box_num = local_COUNT_BOX - 1;

        // чем ближе к центру корзины, тем больше в корзину попадает
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
//        for (int i = 0; i < descriptor->vector->size(); i++) {
//            sum += descriptor->vector->at(i) * descriptor->vector->at(i);
//        }
//        sum = Math.sqrt(sum);
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

    static vector<pair<Descriptor*, Descriptor*>*>* createPairs(Descriptors_turn* first, Descriptors_turn* second) {
        vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* pairsList = new vector<pair<pair<Descriptor*, Descriptor*>*, double>*>;

        for (int i = 0; i < first->descriptorList->size(); i++) {

            Descriptor* firstDescriptor = first->descriptorList->at(i);
            Descriptor* secondDescriptor = NULL;
            int indexSecond = -1;
            double distance = 9999999.0;

            vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* distanceList = new vector<pair<pair<Descriptor*, Descriptor*>*, double>*>;
            // Ищем ближайщий дескриптор для текущего
            for (int j = 0; j < second->descriptorList->size(); j++) {
                Descriptor* tmpSecondDescriptor = second->descriptorList->at(j);


                double tmpDistance = 0.0;

                for (int x = 0; x < firstDescriptor->vector->size(); x++) {
                    tmpDistance += (firstDescriptor->vector->at(x) - tmpSecondDescriptor->vector->at(x)) *
                        (firstDescriptor->vector->at(x) - tmpSecondDescriptor->vector->at(x));
                }
                tmpDistance = sqrt(tmpDistance);

                distanceList->push_back(
                    new pair<pair<Descriptor*, Descriptor*>*, double>(
                        new pair<Descriptor*, Descriptor*>(firstDescriptor, tmpSecondDescriptor),
                        tmpDistance));

            }
            //distanceList.sort(Comparator.comparingdouble(pair::getValue));
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
            if (nndr < 0.8)
                pairsList->push_back(new pair<pair<Descriptor*, Descriptor*>*, double>(
                    new pair<Descriptor*, Descriptor*>(firstDescriptor, secondDescriptor), distance));
        }

        //vector<pair<pair<Descriptor*, Descriptor*>*, double>*>* pairsList
        //pairsList.sort(Comparator.comparingdouble(pair::getValue));

        sort(pairsList->begin(), pairsList->end(),
            [](const pair<pair<Descriptor*, Descriptor*>*, double>* first,
                const pair<pair<Descriptor*, Descriptor*>*, double>* second) {
                    return first->second < second->second;
            });

        vector<pair<Descriptor*, Descriptor*>*>* result = new vector<pair<Descriptor*, Descriptor*>*>;

        // убираем лишнии соответствия
        int indexOk = 0;
        double LEVEL = 0.05;

        for (int i = 2; i < pairsList->size(); i++) {
            double div = pairsList->at(i)->second / pairsList->at(i - 1)->second;
            indexOk = i;
            if (pairsList->at(i)->second > LEVEL)
                break;
        }

        // дебаг
        for (int i = 0; i < pairsList->size(); i++) {
            if (i <= indexOk) {
                result->push_back(pairsList->at(i)->first);
                cout << pairsList->at(i)->second  << "  -  Добавили"  <<  endl;
            }
            else {
                cout << pairsList->at(i)->second << endl;
            }
        }
        cout << "\nresult.size() = " + to_string(result->size())   << "\n\n" << endl;
        return result;
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


    // Рисуем линии
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

