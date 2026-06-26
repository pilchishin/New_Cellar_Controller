#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =================================================================
// 1. СИСТЕМНЫЕ НАСТРОЙКИ И ОТЛАДКА
// =================================================================
// #define DEBUG            // Включение вывода отладочных сообщений в последовательный порт (Serial)
#define SERIAL_SPEED 9600   // Скорость передачи данных Serial-порта
#define WDT_TIMEOUT WDTO_8S // Тайм-аут сторожевого таймера (перезагрузка при зависании через 8 секунд)

// =================================================================
// 2. НАЗНАЧЕНИЕ ПИНОВ (Hardware Mapping)
// =================================================================

// Шина I2C для Arduino Nano: A4 (SDA), A5 (SCL) - задействована аппаратно
// Датчики BME280, HTU21D, RTC DS3231 и LCD 1602 висят на этой шине.
#define BME280_ADDR 0x76

// Пины для кнопок управления
#define BT_UP    3  // Кнопка ВВЕРХ
#define BT_DOWN  4  // Кнопка ВНИЗ
#define BT_MENU  5  // Кнопка МЕНЮ / ВЫБОР

// Пины для SSR (твердотельных реле)
#define PIN_RELAY_FAN   6   // Реле приточного вентилятора
#define PIN_RELAY_OZONE 7   // Реле озонатора

// Пин для шины OneWire (Датчик DS18B20)
#define ONE_WIRE_BUS    2   // Контрольный датчик температуры подвала

// =================================================================
// 3. УСТАВКИ КЛИМАТ-КОНТРОЛЯ (Согласно ТЗ)
// =================================================================
constexpr float kDefaultTargetTemp = 4.0f;     // Целевая температура в подвале по умолчанию (°C)
constexpr float kDefaultTargetRh   = 85.0f;    // Целевая относительная влажность по умолчанию (%)

constexpr float kHysteresisTemp = 0.5f; // Гистерезис по температуре: применяется симметрично для включения и выключения
constexpr float kHysteresisRh   = 3.0f; // Допустимое отклонение влажности от заданной (+/- 3%)

constexpr float kMarginCondSafety = 2.0f; // Безопасный отступ от точки росы для предотвращения конденсата
constexpr float kMarginAh          = 1.0f; // Минимальная разница абсолютной влажности для эффективного проветривания

// =================================================================
// 4. КРИТИЧЕСКИЕ ПОКАЗАТЕЛИ И ОШИБКИ
// =================================================================
constexpr float kTempCriticalMin = 2.0f;  // Минимально допустимая T в подвале (ниже — ошибка)
constexpr float kVentTempMin = 3.0f; // Минимальная T внутри для работы вентилятора
constexpr float kOutTempFrostLimit = 0.0f; // Минимальная T уличного воздуха для вентиляции (°C)
constexpr float kCondensationErrDiff = 0.5f; // Ошибка при риске конденсации (точка росы >= T_внутр - 0.5)
constexpr float kSensorDiffMax = 2.0f;    // Макс. разница между BME280 и DS18B20

// =================================================================
// 5. ТАЙМИНГИ И ИНТЕРВАЛЫ
// =================================================================
constexpr uint32_t kSensorPollInterval = 10000UL; // Опрос датчиков раз в 10 секунд
constexpr uint32_t kSensorRetryInterval = 60000UL; // Интервал повторной попытки опроса датчика
constexpr uint32_t kSensorDeadRetryInterval = 3600000UL; // Попытка повторной инициализации "мертвого" датчика раз в час.
constexpr uint8_t  kSensorMaxRetries    = 3;       // Макс. количество попыток оживления датчика
constexpr uint8_t  kSensorStabilityThreshold = 3;  // Кол-во циклов для стабилизации показаний
constexpr uint32_t kBacklightTimeout    = 60000UL;  // Автовыключение подсветки (60 секунд)
constexpr uint32_t kUserPresenceTimeout = 1800000UL; // Таймаут присутствия пользователя (30 минут)

static_assert(kUserPresenceTimeout >= 300000UL,
"Таймаут присутствия пользователя должен быть не менее 5 минут");

// Интервалы цикла озонирования
constexpr uint32_t kOzoneWorkTime = 900000UL;   // Работа озонатора (15 минут)
constexpr uint32_t kOzoneHoldTime = 7200000UL;  // Пауза/ожидание (2 часа)
constexpr uint32_t kOzoneVentTime = 900000UL;   // Проветривание после (15 минут)

// Повторные попытки при запрете озонирования (мороз на улице)
constexpr uint32_t kOzoneRetryShort = 1800000UL; // Перезапуск через 30 минут
constexpr uint8_t kOzoneMaxRetriesPerDay = 6;

// =================================================================
// 6. ПАРАМЕТРЫ ФИЛЬТРАЦИИ
// =================================================================
constexpr float kFilterEmaAlpha = 0.2f; // Коэффициент сглаживания EMA
constexpr uint8_t kMedianWindow  = 3;    // Размер окна медианного фильтра

// =================================================================
// 7. ГРАНИЦЫ ДОСТОВЕРНОСТИ ДАННЫХ (Plausibility Checks)
// =================================================================
constexpr float kRawTempMin = -40.0f;
constexpr float kRawTempMax = 85.0f;
constexpr float kRawHumMin  = 0.0f;
constexpr float kRawHumMax  = 100.0f;

// --- Интервалы времени (мс) ---
constexpr uint32_t kRtcPollInterval      = 1000UL;
constexpr uint32_t kDeferredSaveDelay    = 30000UL;

// --- Тайминги кнопок (мс) ---
constexpr uint16_t kDebounceMs   = 50;
constexpr uint16_t kRepeatMs     = 150;
constexpr uint16_t kLongPressMs  = 600;
constexpr uint32_t kButtonStuckTimeout = 30000UL;

// --- Защита реле ---
constexpr uint32_t kFanDebounceDelay = 300000UL;
constexpr uint32_t kErrorResetHoldMs = 3000UL;

// --- TimeManager ---
constexpr uint16_t kMinValidYear = 2023;

#endif // CONFIG_H
