#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <Arduino.h>
#include "config.h"

/**
 * @enum SystemState
 * @brief Возможные состояния конечного автомата (FSM) системы.
 */
enum class SystemState {
    kIdle,               // Режим ожидания
    kAutoClimate,       // Автоматический климат-контроль (поддержание T и H)
    kOzoneStart,
    kOzoneActive,
    kOzoneHold,
    kOzoneVent,
    kOzoneAbort,
    kManualFan,
    kManualOzone,
    kErrorState
};

/**
 * @enum ErrorCode
 * @brief Коды ошибок системы для диагностики.
 */
enum class ErrorCode {
    kNone = 0,           // Ошибок нет
    kSensorBmeFail,    // Отказ датчика BME280 (внутренний)
    kSensorHtuFail,
    kSensorDsFail,
    kRtcFail,
    kTempMismatch,      // Расхождение BME280 vs DS18B20 > 2°C
    kCondensationRisk,  // Точка росы >= T - 4°C
    kTempTooLow        // T <= 2°C
};

/**
 * @struct SensorData
 * @brief Структура для хранения обработанных данных с одного датчика климата.
 */
struct SensorData {
    float temp;         // Температура (°C)
    float rh;           // Относительная влажность (%)
    float ah;           // Абсолютная влажность (г/м³)
    float dewpoint;     // Точка росы (°C)
    bool valid;         // Флаг валидности данных (удачное чтение и инициализация)
};

struct CalibrationData {
    float bmeTempOffset;
    float bmeHumOffset;
    float htuTempOffset;
    float htuHumOffset;
    float dsTempOffset;
};

struct SystemStatistics {
    uint32_t uptimeMinutes;
    uint32_t fanMinutes;
    uint32_t ozoneMinutes;
};

#ifdef DEBUG
// Вспомогательные функции для текстового представления (для Serial debug)
inline const __FlashStringHelper* StateToString(SystemState s) {
    switch (s) {
        case SystemState::kIdle:         return F("IDLE");
        case SystemState::kAutoClimate: return F("AUTO");
        case SystemState::kOzoneStart:  return F("O3 START");
        case SystemState::kOzoneActive: return F("O3 WORK");
        case SystemState::kOzoneHold:   return F("O3 HOLD");
        case SystemState::kOzoneVent:   return F("O3 VENT");
        case SystemState::kOzoneAbort:  return F("O3 ABORT");
        case SystemState::kManualFan:   return F("MAN FAN");
        case SystemState::kManualOzone: return F("MAN O3");
        case SystemState::kErrorState:  return F("ERROR");
        default:                        return F("UNKNOWN");
    }
}

inline const __FlashStringHelper* ErrorToString(ErrorCode e) {
    switch (e) {
        case ErrorCode::kNone:              return F("OK");
        case ErrorCode::kSensorBmeFail:   return F("BME FAIL");
        case ErrorCode::kSensorHtuFail:   return F("HTU FAIL");
        case ErrorCode::kSensorDsFail:    return F("DS FAIL");
        case ErrorCode::kRtcFail:          return F("RTC FAIL");
        case ErrorCode::kTempMismatch:     return F("T MISMATCH");
        case ErrorCode::kCondensationRisk: return F("COND RISK");
        case ErrorCode::kTempTooLow:      return F("FROST ERR");
        default:                           return F("ERR");
    }
}
#endif

#endif