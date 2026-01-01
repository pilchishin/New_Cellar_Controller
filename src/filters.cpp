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

// Явная специализация для float (частичная реализация)
template class MedianFilter<float>;
template class EMAFilter<float>;

// Также добавим специализацию для int, если понадобится
template class MedianFilter<int>;
template class EMAFilter<int>;