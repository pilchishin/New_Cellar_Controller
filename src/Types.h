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

#endif