#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <Arduino.h>

/**
 * @class IFilter
 * @brief Интерфейс для фильтров данных датчиков.
 */
class IFilter {
 public:
  virtual ~IFilter() {}
  virtual float Update(float v) = 0;
  virtual void Reset(float initial_value = 0.0f) = 0;
};

/**
 * @class EmaMedianFilter
 * @brief Комбинированный фильтр: медиана (окно 3) + экспоненциальное сглаживание (EMA).
 */
class EmaMedianFilter : public IFilter {
 private:
  float history_[3];     // Буфер для последних 3-х измерений (для медианы)
  float ema_value_;      // Текущее значение EMA
  float alpha_;          // Коэффициент сглаживания EMA (обычно 0.2)
  bool is_initialized_;  // Флаг первоначального заполнения буфера

  // Приватный метод для быстрого поиска медианы из 3-х чисел
  float GetMedian(float a, float b, float c);

 public:
  // Конструктор. По ТЗ коэффициент альфа для EMA равен 0.2
  EmaMedianFilter(float alpha = 0.2f);

  // Главный метод: принимает новое сырое значение, возвращает отфильтрованное
  float Update(float new_value) override;

  // Сброс фильтра (для очистки истории после восстановления датчика)
  void Reset(float initial_value = 0.0f) override;
};

/**
 * @class KalmanFilter
 * @brief Легкий одномерный фильтр Калмана для сглаживания показаний.
 */
class KalmanFilter : public IFilter {
 private:
  float q_;             // Шум процесса (Process noise)
  float r_;             // Шум измерения (Measurement noise)
  float x_;             // Текущая оценка значения (State estimate)
  float p_;             // Ошибка оценки (Estimation error)
  float k_;             // Коэффициент усиления Калмана (Kalman gain)
  bool is_initialized_; // Флаг первого измерения

 public:
  /**
   * @param q Шум процесса. Чем меньше, тем больше доверия модели (фильтр "медленнее").
   * @param r Шум измерения. Чем больше, тем больше фильтрация шумов датчика.
   */
  KalmanFilter(float q = 0.01f, float r = 0.1f);

  float Update(float measurement) override;
  void Reset(float initial_value = 0.0f) override;
};

// Пространство имен для климатических формул
namespace climate_math {
// Расчет абсолютной влажности (г/м³)
float CalculateAH(float temp, float rh);

// Расчет точки росы (°C)
float CalculateDewPoint(float temp, float rh);
}  // namespace climate_math

// Утилиты для работы с шиной I2C
namespace i2c_utils {
// Программный сброс зависшей шины I2C (9 тактов SCL)
void RecoverBus(uint8_t sda_pin, uint8_t scl_pin);
}  // namespace i2c_utils

#endif