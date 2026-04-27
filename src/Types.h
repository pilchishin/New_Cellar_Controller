#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

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
    kTempMismatch,      // BME280 vs DS18B20 > 2C
    kCondensationRisk,  // dewpoint >= T - 4C
    kTempTooLow        // T <= 2C
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

// Вспомогательные функции для текстового представления (для LCD)
inline const char* StateToString(SystemState s) {
    switch (s) {
        case SystemState::kIdle:         return "IDLE";
        case SystemState::kAutoClimate: return "AUTO";
        case SystemState::kOzoneStart:  return "O3 START";
        case SystemState::kOzoneActive: return "O3 WORK";
        case SystemState::kOzoneHold:   return "O3 HOLD";
        case SystemState::kOzoneVent:   return "O3 VENT";
        case SystemState::kManualFan:   return "MAN FAN";
        case SystemState::kManualOzone: return "MAN O3";
        case SystemState::kErrorState:  return "ERROR";
        default:                        return "UNKNOWN";
    }
}

inline const char* ErrorToString(ErrorCode e) {
    switch (e) {
        case ErrorCode::kNone:              return "OK";
        case ErrorCode::kSensorBmeFail:   return "BME FAIL";
        case ErrorCode::kSensorHtuFail:   return "HTU FAIL";
        case ErrorCode::kSensorDsFail:    return "DS FAIL";
        case ErrorCode::kRtcFail:          return "RTC FAIL";
        case ErrorCode::kTempMismatch:     return "T MISMATCH";
        case ErrorCode::kCondensationRisk: return "COND RISK";
        case ErrorCode::kTempTooLow:      return "FROST ERR";
        default:                           return "ERR";
    }
}

#endif