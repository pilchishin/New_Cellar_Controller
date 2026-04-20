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
  virtual void Reset() = 0;
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
  void Reset() override;
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