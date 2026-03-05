#ifndef TYPES_H
#define TYPES_H

enum class SystemState {
    IDLE,
    AUTO_CLIMATE,
    OZONE_START,
    OZONE_ACTIVE,
    OZONE_HOLD,
    OZONE_VENT,
    OZONE_ABORT,
    MANUAL_FAN,
    MANUAL_OZONE,
    ERROR_STATE
};

enum class ErrorCode {
    NONE = 0,
    SENSOR_BME_FAIL,
    SENSOR_HTU_FAIL,
    SENSOR_DS_FAIL,
    RTC_FAIL,
    TEMP_MISMATCH,      // BME280 vs DS18B20 > 2C
    CONDENSATION_RISK,  // dewpoint >= T - 4C
    TEMP_TOO_LOW        // T <= 2C
};

struct SensorData {
    float temp;
    float rh;
    float ah;
    float dewpoint;
    bool valid;
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
inline const char* stateToString(SystemState s) {
    switch (s) {
        case SystemState::IDLE:         return "IDLE";
        case SystemState::AUTO_CLIMATE: return "AUTO";
        case SystemState::OZONE_START:  return "O3 START";
        case SystemState::OZONE_ACTIVE: return "O3 WORK";
        case SystemState::OZONE_HOLD:   return "O3 HOLD";
        case SystemState::OZONE_VENT:   return "O3 VENT";
        case SystemState::OZONE_ABORT:  return "O3 ABORT";
        case SystemState::MANUAL_FAN:   return "MAN FAN";
        case SystemState::MANUAL_OZONE: return "MAN O3";
        case SystemState::ERROR_STATE:  return "ERROR";
        default:                        return "UNKNOWN";
    }
}

inline const char* errorToString(ErrorCode e) {
    switch (e) {
        case ErrorCode::NONE:              return "OK";
        case ErrorCode::SENSOR_BME_FAIL:   return "BME FAIL";
        case ErrorCode::SENSOR_HTU_FAIL:   return "HTU FAIL";
        case ErrorCode::SENSOR_DS_FAIL:    return "DS FAIL";
        case ErrorCode::RTC_FAIL:          return "RTC FAIL";
        case ErrorCode::TEMP_MISMATCH:     return "T MISMATCH";
        case ErrorCode::CONDENSATION_RISK: return "COND RISK";
        case ErrorCode::TEMP_TOO_LOW:      return "FROST ERR";
        default:                           return "ERR";
    }
}

#endif