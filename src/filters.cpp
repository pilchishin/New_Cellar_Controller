#include "filters.h"

// Вспомогательная функция для копирования данных (вместо memcpy)
template<typename T>
void copyBuffer(T* dest, const T* src, int count) {
    for (int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

// Реализация шаблонных методов для шаблонных классов
template<typename T>
MovingAverageFilter<T>::MovingAverageFilter(int windowSize) : Filter<T>() {
    this->windowSize = windowSize;
    buffer = new T[windowSize];
    index = 0;
    count = 0;
    sum = T(0);
}

template<typename T>
T MovingAverageFilter<T>::apply(T input) {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    if (count < windowSize) {
        buffer[index] = input;
        sum += input;
        count++;
    } else {
        sum -= buffer[index];
        buffer[index] = input;
        sum += input;
    }
    index = (index + 1) % windowSize;
    T result = sum / T(count);
    this->lastOutput = result;
    return result;
}

template<typename T>
void MovingAverageFilter<T>::reset() {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    index = 0;
    count = 0;
    sum = T(0);
    for(int i = 0; i < windowSize; i++) {
        buffer[i] = T(0);
    }
}

template<typename T>
MedianFilter<T>::MedianFilter(int windowSize) : Filter<T>() {
    this->windowSize = windowSize;
    buffer = new T[windowSize];
    index = 0;
    count = 0;
}

template<typename T>
T MedianFilter<T>::apply(T input) {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    if (count < windowSize) {
        buffer[count] = input;
        count++;
    } else {
        buffer[index] = input;
        index = (index + 1) % windowSize;
    }

    // Создаем копию буфера для сортировки
    T* tempBuffer = new T[count];
    if (count < windowSize) {
        copyBuffer(tempBuffer, buffer, count);
    } else {
        // Копируем с учетом циклического буфера
        for (int i = 0; i < windowSize; i++) {
            tempBuffer[i] = buffer[(index + i) % windowSize];
        }
    }

    // Сортируем
    sortBuffer(tempBuffer, count);

    // Находим медиану
    T result;
    if (count % 2 == 1) {
        result = tempBuffer[count / 2];
    } else {
        result = (tempBuffer[count / 2 - 1] + tempBuffer[count / 2]) / T(2);
    }

    delete[] tempBuffer;
    this->lastOutput = result;
    return result;
}

template<typename T>
void MedianFilter<T>::reset() {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    index = 0;
    count = 0;
    for(int i = 0; i < windowSize; i++) {
        buffer[i] = T(0);
    }
}

template<typename T>
void MedianFilter<T>::sortBuffer(T* arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                T temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

template<typename T>
EMAFilter<T>::EMAFilter(T alpha) : Filter<T>() {
    this->alpha = alpha;
    this->initialized = false;
}

template<typename T>
T EMAFilter<T>::apply(T input) {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    T result;
    if (!this->initialized) {
        result = input;
        this->lastOutput = result;
        this->initialized = true;
    } else {
        result = this->alpha * input + (T(1) - this->alpha) * this->lastOutput;
        this->lastOutput = result;
    }
    return result;
}

template<typename T>
void EMAFilter<T>::reset() {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    this->lastOutput = T(0);
    this->initialized = false;
}

template<typename T>
KalmanFilter<T>::KalmanFilter(T processNoise, T measurementNoise) : Filter<T>() {
    this->processNoise = processNoise;
    this->measurementNoise = measurementNoise;
    this->estimate = T(0);
    this->error = T(0);
    this->kalmanGain = T(0);
}

template<typename T>
T KalmanFilter<T>::apply(T input) {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    return T(0); // Заглушка - полная реализация требует более сложной логики
}

template<typename T>
void KalmanFilter<T>::reset() {
#ifdef THREAD_SAFE_FILTERS
    std::lock_guard<std::mutex> lock(this->mtx);
#endif
    this->estimate = T(0);
    this->error = T(0);
    this->kalmanGain = T(0);
}

// Явная специализация для float (частичная реализация)
template class MovingAverageFilter<float>;
template class MedianFilter<float>;
template class EMAFilter<float>;
template class KalmanFilter<float>;

// Также добавим специализацию для int, если понадобится
template class MovingAverageFilter<int>;
template class MedianFilter<int>;
template class EMAFilter<int>;
template class KalmanFilter<int>;