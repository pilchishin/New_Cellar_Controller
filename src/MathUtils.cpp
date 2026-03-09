#include "MathUtils.h"
#include <math.h> // Необходима для функций exp() и log()

// Инициализация класса фильтра
Filter::Filter(float alpha) {
  this->alpha_ = alpha;
  this->is_initialized_ = false;
  this->ema_value_ = 0.0f;
  for (int i = 0; i < 3; i++) {
    this->history_[i] = 0.0f;
  }
}

/**
 * @brief Быстрый алгоритм поиска медианы для трех чисел без полноценной сортировки массива.
 */
float Filter::GetMedian(float a, float b, float c) {
  return max(min(a, b), min(max(a, b), c));
}

// Обновление значения в фильтре
float Filter::Update(float new_value) {
  // Если это самое первое измерение после включения устройства
  if (!is_initialized_) {
    // Заполняем весь буфер первым валидным значением
    history_[0] = history_[1] = history_[2] = new_value;
    ema_value_ = new_value;
    is_initialized_ = true;
    return new_value;
  }

  // 1. МЕДИАННЫЙ ФИЛЬТР
  history_[0] = history_[1];
  history_[1] = history_[2];
  history_[2] = new_value;

  float median = GetMedian(history_[0], history_[1], history_[2]);

  // 2. ФИЛЬТР EMA
  ema_value_ = (alpha_ * median) + ((1.0f - alpha_) * ema_value_);

  return ema_value_;
}

// ==========================================================
// КЛИМАТИЧЕСКАЯ МАТЕМАТИКА
// ==========================================================

/**
 * @brief Расчет абсолютной влажности (г/м³) по формуле Магнуса-Тетенса.
 */
float climate_math::CalculateAH(float temp, float rh) {
  // svp - Saturation Vapor Pressure (давление насыщенного пара, гПа)
  float svp = 6.112f * exp((17.62f * temp) / (243.12f + temp));

  // vp - Actual Vapor Pressure (фактическое давление пара, гПа)
  float vp = (rh / 100.0f) * svp;

  // ah - Absolute Humidity
  float ah = 216.7f * vp / (temp + 273.15f);

  return ah;
}

/**
 * @brief Расчет точки росы (°C).
 */
float climate_math::CalculateDewPoint(float temp, float rh) {
  // Защита от логарифма нуля
  if (rh <= 0.0f) rh = 0.1f;

  // Промежуточная переменная H (gamma) на основе обратной формулы Магнуса
  float H = log(rh / 100.0f) + ((17.62f * temp) / (243.12f + temp));

  // Расчет финальной температуры точки росы (°C)
  float dew_point = (243.12f * H) / (17.62f - H);

  return dew_point;
}

// ==========================================================
// I2C UTILS
// ==========================================================
#include <Wire.h>

void i2c_utils::RecoverBus(uint8_t sda_pin, uint8_t scl_pin) {
    #ifdef DEBUG
    Serial.println(F("I2C: Recovery procedure started..."));
    #endif

    // 1. Освобождаем шину (отключаем аппаратный I2C)
    Wire.end();

    // 2. Настраиваем пины на вывод
    pinMode(sda_pin, INPUT_PULLUP);
    pinMode(scl_pin, OUTPUT);
    digitalWrite(scl_pin, HIGH);

    // 3. Отправляем 9 импульсов SCL
    // Это заставит любое устройство, зависшее в ожидании ACK, освободить SDA
    for (int i = 0; i < 9; i++) {
        digitalWrite(scl_pin, LOW);
        delayMicroseconds(5);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);

        // Если SDA освободился (стал HIGH), можно закончить раньше
        if (digitalRead(sda_pin) == HIGH && i > 0) {
            #ifdef DEBUG
            Serial.print(F("I2C: Bus released at cycle ")); Serial.println(i);
            #endif
            break;
        }
    }

    // 4. Формируем сигнал STOP: SDA low->high пока SCL high
    pinMode(sda_pin, OUTPUT);
    digitalWrite(sda_pin, LOW);
    delayMicroseconds(5);
    digitalWrite(sda_pin, HIGH);
    delayMicroseconds(5);

    // 5. Возвращаем аппаратный I2C
    Wire.begin();

    #ifdef DEBUG
    Serial.println(F("I2C: Recovery complete."));
    #endif
}