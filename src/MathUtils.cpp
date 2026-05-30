#include "MathUtils.h"
#include <math.h> // Необходима для функций exp() и log()

/**
 * @brief Инициализация комбинированного фильтра.
 * @param alpha Коэффициент сглаживания EMA.
 */
EmaMedianFilter::EmaMedianFilter(float alpha) {
  this->alpha_ = alpha;
  // Коэффициент Alpha ограничивается диапазоном (0, 1) исключительно для предотвращения вырождения фильтра.
  if (this->alpha_ <= 0.0f) this->alpha_ = 0.01f;
  if (this->alpha_ >= 1.0f) this->alpha_ = 0.99f;

  this->is_initialized_ = false;
  this->ema_value_ = 0.0f;
  for (int i = 0; i < 3; i++) {
    this->history_[i] = 0.0f;
  }
}

/**
 * @brief Быстрый алгоритм поиска медианы для трех чисел.
 * Не требует сортировки массива, реализован через логические сравнения.
 */
float EmaMedianFilter::GetMedian(float a, float b, float c) {
  return max(min(a, b), min(max(a, b), c));
}

/**
 * @brief Обработка нового измерения.
 * Сначала применяется медианный фильтр для удаления выбросов,
 * затем результат сглаживается методом EMA.
 */
float EmaMedianFilter::Update(float new_value) {
  // Инициализация при первом запуске (заполнение всей истории первым значением)
  if (!is_initialized_) {
    history_[0] = history_[1] = history_[2] = new_value;
    ema_value_ = new_value;
    is_initialized_ = true;
    return new_value;
  }

  // 1. Обновление истории для медианного фильтра (сдвиг окна)
  history_[0] = history_[1];
  history_[1] = history_[2];
  history_[2] = new_value;

  // Поиск медианы в окне из 3-х элементов
  float median = GetMedian(history_[0], history_[1], history_[2]);

  // 2. Экспоненциальное сглаживание полученной медианы
  ema_value_ = (alpha_ * median) + ((1.0f - alpha_) * ema_value_);

  return ema_value_;
}

/**
 * @brief Принудительный сброс фильтра на заданное значение.
 */
void EmaMedianFilter::Reset(float initial_value) {
  this->is_initialized_ = true;
  this->ema_value_ = initial_value;
  for (int i = 0; i < 3; i++) {
    this->history_[i] = initial_value;
  }
}

/**
 * @brief Инвалидация (аннулирование) состояния фильтра.
 */
void EmaMedianFilter::Invalidate() {
  this->is_initialized_ = false;
}

// ==========================================================
// КЛИМАТИЧЕСКАЯ МАТЕМАТИКА
// ==========================================================

/**
 * @brief Расчет абсолютной влажности (г/м³).
 * Использует аппроксимацию формулы Магнуса-Тетенса.
 */
float climate_math::CalculateAH(float temp, float rh) {
  // Расчет давления насыщенного пара (svp - Saturation Vapor Pressure, гПа)
  float svp = 6.112f * exp((17.62f * temp) / (243.12f + temp));

  // Расчет фактического давления пара (vp - Actual Vapor Pressure, гПа)
  float vp = (rh / 100.0f) * svp;

  // Расчет абсолютной влажности (ah - Absolute Humidity, г/м³)
  float ah = 216.7f * vp / (temp + 273.15f);

  return ah;
}

/**
 * @brief Расчет точки росы (°C).
 * Температура, при которой водяной пар в воздухе становится насыщенным.
 */
float climate_math::CalculateDewPoint(float temp, float rh) {
  // Защита от математической ошибки (логарифм нуля)
  if (rh <= 0.0f) rh = 0.1f;

  // Промежуточный расчет коэффициента гамма (H)
  float H = log(rh / 100.0f) + ((17.62f * temp) / (243.12f + temp));

  // Финальный расчет температуры точки росы (°C)
  float dew_point = (243.12f * H) / (17.62f - H);

  return dew_point;
}

// ==========================================================
// УТИЛИТЫ ШИНЫ I2C
// ==========================================================
#include <Wire.h>

/**
 * @brief Процедура программного освобождения шины I2C.
 * Если ведомое устройство зависло в процессе передачи (держит SDA в LOW),
 * мастер генерирует до 9 импульсов синхронизации для завершения транзакции.
 *
 * @note Предусловие: Вызывающий должен вызвать Wire.end() перед этой функцией.
 * Эта функция только выполняет последовательность импульсов bit-bang и STOP.
 */
void i2c_utils::RecoverBus(uint8_t sda_pin, uint8_t scl_pin) {
    #ifdef DEBUG
    Serial.println(F("I2C: Запуск процедуры восстановления шины..."));
    #endif

    // 1. Перевод пинов в режим программного управления (Bit-bang)
    pinMode(sda_pin, INPUT_PULLUP);
    pinMode(scl_pin, OUTPUT);
    digitalWrite(scl_pin, HIGH);

    // 3. Генерация 9 импульсов SCL
    for (int i = 0; i < 9; i++) {
        digitalWrite(scl_pin, LOW);
        delayMicroseconds(5);
        digitalWrite(scl_pin, HIGH);
        delayMicroseconds(5);

        // Проверка: если линия SDA освобождена (стала HIGH), выходим раньше
        if (digitalRead(sda_pin) == HIGH && i > 0) {
            #ifdef DEBUG
            Serial.print(F("I2C: Шина освобождена на цикле ")); Serial.println(i);
            #endif
            break;
        }
    }

    // 4. Формирование сигнала STOP (SDA: LOW -> HIGH при SCL: HIGH)
    pinMode(sda_pin, OUTPUT);
    digitalWrite(sda_pin, LOW);
    delayMicroseconds(5);
    digitalWrite(sda_pin, HIGH);
    delayMicroseconds(5);

    #ifdef DEBUG
    Serial.println(F("I2C: Восстановление завершено."));
    #endif
}
