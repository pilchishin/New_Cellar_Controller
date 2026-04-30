#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <Arduino.h>

/**
 * @class IFilter
 * @brief Общий интерфейс для алгоритмов фильтрации данных датчиков.
 * Позволяет унифицировать обработку зашумленных значений температуры и влажности.
 */
class IFilter {
 public:
  virtual ~IFilter() {}

  /**
   * @brief Добавление нового измерения в фильтр.
   * @param v Сырое значение с датчика.
   * @return Отфильтрованное значение.
   */
  virtual float Update(float v) = 0;

  /**
   * @brief Принудительная установка состояния фильтра.
   * @param initial_value Значение, которым будет заполнена история.
   */
  virtual void Reset(float initial_value = 0.0f) = 0;

  /**
   * @brief Сброс состояния инициализации.
   * Следующий вызов Update() выполнит Reset() автоматически.
   */
  virtual void Invalidate() = 0;

  /**
   * @brief Проверка, накоплено ли достаточно данных для работы.
   */
  virtual bool IsInitialized() const = 0;
};

/**
 * @class EmaMedianFilter
 * @brief Гибридный нелинейный фильтр.
 * Сочетает медианный фильтр (окно 3) для отсечения резких выбросов
 * и экспоненциальное скользящее среднее (EMA) для плавного сглаживания шума.
 */
class EmaMedianFilter : public IFilter {
 private:
  float history_[3];     ///< Буфер для последних 3-х измерений (окно медианы)
  float ema_value_;      ///< Накопленное сглаженное значение
  float alpha_;          ///< Коэффициент веса нового значения (0.0 - 1.0)
  bool is_initialized_;  ///< Флаг готовности фильтра

  /**
   * @brief Оптимизированный поиск медианы из трех чисел.
   */
  float GetMedian(float a, float b, float c);

 public:
  /**
   * @brief Конструктор фильтра.
   * @param alpha Коэффициент сглаживания (по умолчанию 0.2).
   */
  EmaMedianFilter(float alpha = 0.2f);

  float Update(float new_value) override;
  void Reset(float initial_value = 0.0f) override;
  void Invalidate() override;
  bool IsInitialized() const override { return is_initialized_; }
};

/**
 * @namespace climate_math
 * @brief Математические функции для расчета производных параметров микроклимата.
 */
namespace climate_math {
  /**
   * @brief Расчет абсолютной влажности (г/м³).
   * Использует формулу Магнуса-Тетенса для определения плотности водяного пара.
   * @param temp Температура (°C).
   * @param rh Относительная влажность (%).
   */
  float CalculateAH(float temp, float rh);

  /**
   * @brief Расчет температуры точки росы (°C).
   * Позволяет определить риск выпадения конденсата на поверхностях.
   * @param temp Температура (°C).
   * @param rh Относительная влажность (%).
   */
  float CalculateDewPoint(float temp, float rh);
}

/**
 * @namespace i2c_utils
 * @brief Низкоуровневые утилиты для обслуживания шины I2C.
 */
namespace i2c_utils {
  /**
   * @brief Программная процедура восстановления шины I2C.
   * Генерирует 9 тактов SCL в режиме bit-bang, чтобы вывести ведомые устройства
   * из состояния ожидания и освободить заблокированную линию SDA.
   * @param sda_pin Номер пина SDA.
   * @param scl_pin Номер пина SCL.
   */
  void RecoverBus(uint8_t sda_pin, uint8_t scl_pin);
}

#endif