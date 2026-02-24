#include "MathUtils.h"
#include <math.h> // Необходима для функций exp() и log()

// Инициализация класса фильтра
Filter::Filter(float alpha) {
    this->alpha = alpha;
    this->isInitialized = false;
    this->emaValue = 0.0f;
    for (int i = 0; i < 3; i++) {
        this->history[i] = 0.0f;
    }
}

// Быстрый алгоритм поиска медианы для трех чисел без полноценной сортировки массива
// Это экономит такты процессора на слабой 16 МГц Arduino Nano
float Filter::getMedian(float a, float b, float c) {
    return max(min(a, b), min(max(a, b), c));
}

// Обновление значения в фильтре
float Filter::update(float newValue) {
    // Если это самое первое измерение после включения устройства
    if (!isInitialized) {
        // Заполняем весь буфер первым валидным значением, чтобы не было "разгона" от нуля
        history[0] = history[1] = history[2] = newValue;
        emaValue = newValue;
        isInitialized = true;
        return newValue;
    }

    // 1. МЕДИАННЫЙ ФИЛЬТР
    // Сдвигаем историю: самое старое удаляем, новое записываем в конец
    history[0] = history[1];
    history[1] = history[2];
    history[2] = newValue;

    // Находим медианное значение из окна в 3 элемента (отсекаем "спайки"/выбросы)
    float median = getMedian(history[0], history[1], history[2]);

    // 2. ФИЛЬТР EMA (Экспоненциальное скользящее среднее)
    // Плавно подтягиваем текущее значение к новой медиане
    // Формула: EMA = (alpha * НовоеЗначение) + ((1 - alpha) * СтароеЗначение)
    emaValue = (alpha * median) + ((1.0f - alpha) * emaValue);

    return emaValue;
}

// ==========================================================
// КЛИМАТИЧЕСКАЯ МАТЕМАТИКА
// ==========================================================

// Расчет абсолютной влажности по формуле Магнуса
float ClimateMath::calculateAH(float temp, float rh) {
    // svp - Saturation Vapor Pressure (давление насыщенного пара, гПа)
    // Используем константы для воды из ТЗ (формула Магнуса-Тетенса)
    float svp = 6.112f * exp((17.62f * temp) / (243.12f + temp));
    
    // vp - Actual Vapor Pressure (фактическое давление пара, гПа)
    float vp = (rh / 100.0f) * svp;
    
    // ah - Absolute Humidity (абсолютная влажность в граммах на кубический метр)
    // Формула перевода давления пара в плотность с учетом постоянной 216.7
    // Для перевода градусов Цельсия в Кельвины прибавляем 273.15
    float ah = 216.7f * vp / (temp + 273.15f);
    
    return ah;
}

// Расчет точки росы (°C)
float ClimateMath::calculateDewPoint(float temp, float rh) {
    // Защита от логарифма нуля (если датчик вернет 0% влажности)
    if (rh <= 0.0f) rh = 0.1f;
    
    // Промежуточная переменная H (gamma) на основе обратной формулы Магнуса
    float H = log(rh / 100.0f) + ((17.62f * temp) / (243.12f + temp));
    
    // Расчет финальной температуры точки росы (°C)
    float dewPoint = (243.12f * H) / (17.62f - H);
    
    return dewPoint;
}