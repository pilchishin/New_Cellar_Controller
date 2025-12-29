#include "sensors.h"
#include <Wire.h>           // Для I2C
#include <Adafruit_Sensor.h>  // Базовая библиотека сенсоров
#include <Adafruit_BME280.h>  // Библиотека BME280

// Библиотеки для DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

// Создаем глобальный объект BME280
Adafruit_BME280 bme;  // I2C

// Глобальные объекты для DS18B20
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

SensorManager::SensorManager() {

    // Инициализация BME280
    bme280Initialized = false;
    bme280Address = BME280_ADDRESS;
    bme280Temperature = 0.0f;
    bme280Humidity = 0.0f;
    bme280TempValid = false;
    bme280HumidityValid = false;
    
    // Инициализация DS18B20
    ds18b20Initialized = false;
    ds18b20Temperature = 0.0f;
    ds18b20TempValid = false;
    // Инициализируем адрес DS18B20 как пустой
    for (int i = 0; i < 8; i++) {
        ds18b20Address[i] = 0;
    }
    
    // Инициализация HTU21D
    htu21dInitialized = false;
    htu21dAddress = HTU21D_ADDRESS;
    htu21dTemperature = 0.0f;
    htu21dHumidity = 0.0f;
    htu21dTempValid = false;
    htu21dHumidityValid = false;
}

void SensorManager::init() {
    // Инициализация сенсоров
    Serial.println("Инициализация сенсоров...");
    
    // Инициализация I2C для BME280
    Wire.begin();
    
    // Попытка инициализации BME280 с основным адресом
    if (!bme.begin(bme280Address)) {
        Serial.print("Не удалось найти BME280 с адресом 0x");
        Serial.println(bme280Address, HEX);
        // Пробуем второй возможный адрес
        bme280Address = 0x77;
        if (!bme.begin(bme280Address)) {
            Serial.print("Не удалось найти BME280 с адресом 0x");
            Serial.println(bme280Address, HEX);
            Serial.println("Проверьте подключение датчика!");
            bme280Initialized = false;
        } else {
            Serial.print("Найден BME280 с адресом 0x");
            Serial.println(bme280Address, HEX);
            bme280Initialized = true;  // Устанавливаем флаг инициализации
        }
    } else {
        Serial.print("Найден BME280 с адресом 0x");
        Serial.println(bme280Address, HEX);
        bme280Initialized = true;  // Устанавливаем флаг инициализации
    }
    
    // Установка параметров датчика BME280
    if (bme.begin(bme280Address)) {  // Проверяем успешность инициализации и устанавливаем флаг
        bme280Initialized = true;
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X2,  // температура
                        Adafruit_BME280::SAMPLING_X1,  // влажность
                        Adafruit_BME280::FILTER_X16,
                        Adafruit_BME280::STANDBY_MS_500);
        Serial.println("BME280 успешно инициализирован!");
    } else {
        Serial.println("BME280 не инициализирован.");
    }
    
    // Инициализация DS18B20
    if (!initDS18B20()) {
        Serial.println("DS18B20 не инициализирован.");
        ds18b20Initialized = false;
    } else {
        ds18b20Initialized = true;  // Устанавливаем флаг инициализации
    }
    
    // Инициализация HTU21D
    if (!initHTU21D()) {
        Serial.println("HTU21D не инициализирован.");
        htu21dInitialized = false;
    } else {
        Serial.println("HTU21D успешно инициализирован.");
        htu21dInitialized = true;  // Устанавливаем флаг инициализации
    }
}

/**
 * @brief Инициализация датчика DS18B20
 *
 * Выполняет полную инициализацию датчика DS18B20 по протоколу 1-Wire:
 * - Сброс шины 1-Wire и проверка присутствия устройства
 * - Поиск идентификация устройства DS18B20
 * - Установка разрешения измерения
 * - Валидация инициализации
 *
 * @return true в случае успешной инициализации, false в случае ошибки
 */
bool SensorManager::initDS18B20() {
    Serial.println("Инициализация DS18B20...");
    
    // Инициализируем библиотеку DallasTemperature
    ds18b20.begin();
    
    // Проверяем количество найденных устройств
    int deviceCount = ds18b20.getDeviceCount();
    Serial.print("Найдено устройств DS18B20: ");
    Serial.println(deviceCount);
    
    if (deviceCount == 0) {
        Serial.println("Не найдено ни одного устройства DS18B20!");
        return false;
    }
    
    // Получаем адрес первого устройства
    if (!ds18b20.getAddress(ds18b20Address, 0)) {
        Serial.println("Не удалось получить адрес устройства DS18B20!");
        return false;
    }
    
    // Проверяем, является ли устройство DS18B20
    if (!ds18b20.validFamily(ds18b20Address)) {
        Serial.println("Устройство не является DS18B20!");
        return false;
    }
    
    // Устанавливаем разрешение измерения
    setDS18B20Resolution();
    
    // Выполняем проверку валидации
    if (!validateDS18B20()) {
        Serial.println("Проверка валидации DS18B20 не пройдена!");
        return false;
    }
    
    Serial.println("DS18B20 успешно инициализирован!");
    ds18b20Initialized = true;
    return true;
}

/**
 * @brief Установка разрешения измерения DS18B20
 *
 * Устанавливает разрешение измерения температуры датчика DS18B20.
 * Максимальное разрешение 12 бит обеспечивает точность 0.0625°C.
 */
void SensorManager::setDS18B20Resolution() {
    // Устанавливаем разрешение измерения на 12 бит для максимальной точности
    ds18b20.setResolution(ds18b20Address, DS18B20_RESOLUTION);
    Serial.print("Установлено разрешение DS18B20: ");
    Serial.print(DS18B20_RESOLUTION);
    Serial.println(" бит");
}

/**
 * @brief Чтение температуры с датчика DS18B20
 *
 * Выполняет чтение температуры с датчика DS18B20.
 * Включает запуск преобразования и получение результата.
 *
 * @param temp Переменная для хранения значения температуры
 * @return true в случае успешного чтения, false в случае ошибки
 */
bool SensorManager::readDS18B20Temp(float& temp) {
    if (!ds18b20Initialized) {
        Serial.println("DS18B20 не инициализирован!");
        return false;
    }
    
    // Запускаем преобразование температуры
    ds18b20.requestTemperatures();
    
    // Читаем температуру с устройства
    temp = ds18b20.getTempC(ds18b20Address);
    
    // Проверяем, является ли значение действительным
    if (temp == DEVICE_DISCONNECTED_C) {
        Serial.println("Ошибка чтения температуры с DS18B20!");
        return false;
    }
    
    return true;
}

/**
 * @brief Валидация датчика DS18B20
 *
 * Выполняет тестовое чтение температуры для проверки работоспособности датчика.
 * Используется для подтверждения корректной инициализации.
 *
 * @return true если датчик прошел валидацию, false в противном случае
 */
bool SensorManager::validateDS18B20() {
    // Выполняем тестовое чтение температуры для валидации
    float testTemp;
    return readDS18B20Temp(testTemp);
}

/**
 * @brief Чтение данных с датчиков
 *
 * Выполняет чтение данных с доступных датчиков температуры и влажности:
 * - BME280 (температура и влажность)
 * - DS18B20 (температура)
 * - HTU21D (температура и влажность)
 *
 * Данные сохраняются отдельно для каждого датчика без объединения.
 */
void SensorManager::readSensors() {
    // Чтение данных с BME280 (температура и влажность)
    if (bme280Initialized) {
        float temp_bme = bme.readTemperature();
        float humidity_bme = bme.readHumidity();
        
        // Проверка валидности данных BME280
        if (!isnan(temp_bme)) {
            bme280Temperature = temp_bme;
            bme280TempValid = true;
        } else {
            bme280TempValid = false;
        }
        
        if (!isnan(humidity_bme)) {
            bme280Humidity = humidity_bme;
            bme280HumidityValid = true;
        } else {
            bme280HumidityValid = false;
        }
    } else {
        bme280TempValid = false;
        bme280HumidityValid = false;
    }
    
    // Чтение данных с DS18B20 (температура)
    if (ds18b20Initialized) {
        float temp_ds18b20;
        if (readDS18B20Temp(temp_ds18b20)) {
            ds18b20Temperature = temp_ds18b20;
            ds18b20TempValid = true;
        } else {
            ds18b20TempValid = false;
        }
    } else {
        ds18b20TempValid = false;
    }
    
    // Чтение данных с HTU21D (температура и влажность)
    if (htu21dInitialized) {
        float temp_htu21d, humidity_htu21d;
        bool tempValid = readHTU21DTemperature(temp_htu21d);
        bool humidityValid = readHTU21DHumidity(humidity_htu21d);
        
        if (tempValid) {
            htu21dTemperature = temp_htu21d;
            htu21dTempValid = true;
        } else {
            htu21dTempValid = false;
        }
        
        if (humidityValid) {
            htu21dHumidity = humidity_htu21d;
            htu21dHumidityValid = true;
        } else {
            htu21dHumidityValid = false;
        }
    } else {
        htu21dTempValid = false;
        htu21dHumidityValid = false;
    }
    
}


/**
 * @brief Инициализация датчика HTU21D
 *
 * Выполняет полную инициализацию датчика влажности и температуры HTU21D через интерфейс I2C:
 * - Проверка соединения с устройством
 * - Сброс устройства при необходимости
 * - Настройка регистра конфигурации
 * - Установка разрешения измерений по умолчанию
 * - Отключение режима удержания при измерениях
 * - Инициализация таймингов для корректного взаимодействия с датчиком
 *
 * @return true в случае успешной инициализации, false в случае ошибки
 */
bool SensorManager::initHTU21D() {
    Serial.println("Инициализация HTU21D...");
    
    // Проверяем соединение с устройством по основному адресу
    Wire.beginTransmission(htu21dAddress);
    if (Wire.endTransmission() != 0) {
        Serial.print("HTU21D не найден по адресу 0x");
        Serial.println(htu21dAddress, HEX);
        // Пробуем альтернативный адрес (если поддерживается)
        htu21dAddress = 0x41;
        Wire.beginTransmission(htu21dAddress);
        if (Wire.endTransmission() != 0) {
            Serial.print("HTU21D не найден по адресу 0x");
            Serial.println(htu21dAddress, HEX);
            Serial.println("Проверьте подключение датчика!");
            return false;
        }
    }
    
    // Выполняем сброс устройства
    if (!resetHTU21D()) {
        Serial.println("Ошибка сброса HTU21D!");
        return false;
    }
    
    // Читаем регистр конфигурации
    uint8_t config = 0;
    if (!readHTU21DRegister(0xE7, config)) {  // Read User Register
        Serial.println("Ошибка чтения регистра конфигурации HTU21D!");
        return false;
    }
    
    // Устанавливаем разрешение измерений (12-bit для влажности и 14-bit для температуры)
    // Биты 7 и 0 определяют разрешение:
    // 0b00: 12-bit RH, 14-bit Temp
    // 0b01: 8-bit RH, 12-bit Temp
    // 0b10: 10-bit RH, 13-bit Temp
    // 0b11: 11-bit RH, 11-bit Temp
    config &= 0x7E;  // Очищаем биты 7 и 0
    config |= 0x00;  // Устанавливаем 12-bit RH, 14-bit Temp (по умолчанию)
    
    // Отключаем режим удержания (hold master)
    // Бит 6 регистра конфигурации: 0 = hold, 1 = no hold
    config |= 0x02;  // Устанавливаем бит 1 в 1 (отключаем hold mode)
    
    // Записываем обновленную конфигурацию
    if (!writeHTU21DRegister(0xE6, config)) {  // Write User Register
        Serial.println("Ошибка записи регистра конфигурации HTU21D!");
        return false;
    }
    
    // Выполняем проверку валидации
    if (!validateHTU21D()) {
        Serial.println("Проверка валидации HTU21D не пройдена!");
        return false;
    }
    
    Serial.println("HTU21D успешно инициализирован!");
    htu21dInitialized = true;
    return true;
}

/**
 * @brief Сброс датчика HTU21D
 *
 * Выполняет программный сброс датчика HTU21D по I2C.
 *
 * @return true в случае успешного сброса, false в случае ошибки
 */
bool SensorManager::resetHTU21D() {
    Serial.println("Сброс HTU21D...");
    
    Wire.beginTransmission(htu21dAddress);
    Wire.write(0xFE);  // Команда сброса
    if (Wire.endTransmission() != 0) {
        Serial.println("Ошибка отправки команды сброса HTU21D!");
        return false;
    }
    
    // После сброса требуется ожидание (до 15 мс)
    // Используем millis() для неблокирующей задержки
    unsigned long startTime = millis();
    while (millis() - startTime < 15) {
        // Неблокирующая задержка
        delay(1);  // Небольшая задержка для предотвращения чрезмерного потребления CPU
    }
    
    Serial.println("HTU21D сброшен успешно!");
    return true;
}

/**
 * @brief Чтение температуры с датчика HTU21D
 *
 * Выполняет чтение температуры с датчика HTU21D.
 *
 * @param temp Переменная для хранения значения температуры
 * @return true в случае успешного чтения, false в случае ошибки
 */
bool SensorManager::readHTU21DTemperature(float& temp) {
    // Проверка валидности входных параметров
    if (!htu21dInitialized) {
        Serial.println("HTU21D не инициализирован!");
        return false;
    }
    
    // Отправляем команду измерения температуры (без удержания)
    Wire.beginTransmission(htu21dAddress);
    Wire.write(0xF3);  // Measure Temperature, No Hold Master Mode
    if (Wire.endTransmission() != 0) {
        Serial.println("Ошибка отправки команды измерения температуры HTU21D!");
        return false;
    }
    
    // Ждем завершения измерения (максимум HTU21D_TEMP_MEASUREMENT_TIME мс для 14-bit разрешения)
    // Используем millis() для неблокирующей задержки
    unsigned long startTime = millis();
    while (millis() - startTime < HTU21D_TEMP_MEASUREMENT_TIME) {
        // Неблокирующая задержка
        delay(1);  // Небольшая задержка для предотвращения чрезмерного потребления CPU
    }
    
    // Читаем результат
    Wire.requestFrom(htu21dAddress, 3);
    if (Wire.available() < 3) {
        Serial.println("Недостаточно данных при чтении температуры HTU21D!");
        return false;
    }
    
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read();  // CRC checksum
    
    // Комбинируем старший и младший байты
    uint16_t rawTemp = (msb << 8) | lsb;
    
    // Проверяем CRC
    if (!checkCRC(rawTemp, crc)) {
        Serial.println("Ошибка CRC при чтении температуры HTU21D!");
        return false;
    }
    
    // Убираем два младших бита (статусные биты)
    rawTemp &= 0xFFFC;
    
    // Преобразуем в температуру (формула из datasheet)
    temp = (rawTemp * 175.72 / 65536.0) - 46.85;
    
    return true;
}
/**
 * @brief Чтение влажности с датчика HTU21D
 *
 * Выполняет чтение влажности с датчика HTU21D.
 *
 * @param humidity Переменная для хранения значения влажности
 * @return true в случае успешного чтения, false в случае ошибки
 */
bool SensorManager::readHTU21DHumidity(float& humidity) {
    // Проверка валидности входных параметров
    if (!htu21dInitialized) {
        Serial.println("HTU21D не инициализирован!");
        return false;
    }
    
    // Отправляем команду измерения влажности (без удержания)
    Wire.beginTransmission(htu21dAddress);
    Wire.write(0xF5);  // Measure Humidity, No Hold Master Mode
    if (Wire.endTransmission() != 0) {
        Serial.println("Ошибка отправки команды измерения влажности HTU21D!");
        return false;
    }
    
    // Ждем завершения измерения (максимум HTU21D_HUM_MEASUREMENT_TIME мс для 12-bit разрешения)
    // Используем millis() для неблокирующей задержки
    unsigned long startTime = millis();
    while (millis() - startTime < HTU21D_HUM_MEASUREMENT_TIME) {
        // Неблокирующая задержка
        delay(1);  // Небольшая задержка для предотвращения чрезмерного потребления CPU
    }
    
    // Читаем результат
    Wire.requestFrom(htu21dAddress, 3);
    if (Wire.available() < 3) {
        Serial.println("Недостаточно данных при чтении влажности HTU21D!");
        return false;
    }
    
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t crc = Wire.read();  // CRC checksum
    
    // Комбинируем старший и младший байты
    uint16_t rawHumidity = (msb << 8) | lsb;
    
    // Проверяем CRC
    if (!checkCRC(rawHumidity, crc)) {
        Serial.println("Ошибка CRC при чтении влажности HTU21D!");
        return false;
    }
    
    // Убираем два младших бита (статусные биты)
    rawHumidity &= 0xFFFC;
    
    // Преобразуем во влажность (формула из datasheet)
    humidity = (rawHumidity * 125.0 / 65536.0) - 6;
    
    return true;
}
// Удалена лишняя закрывающая скобка

/**
 * @brief Валидация датчика HTU21D
 *
 * Выполняет тестовое чтение температуры и влажности для проверки работоспособности датчика.
 * Используется для подтверждения корректной инициализации.
 *
 * @return true если датчик прошел валидацию, false в противном случае
 */
bool SensorManager::validateHTU21D() {
    Serial.println("Валидация HTU21D...");
    
    float testTemp, testHumidity;
    
    // Выполняем тестовое чтение температуры и влажности с повторными попытками
    bool tempReadSuccess = false;
    bool humidityReadSuccess = false;
    
    // Попытка чтения температуры (до 3 попыток)
    for (int attempt = 0; attempt < 3 && !tempReadSuccess; attempt++) {
        if (readHTU21DTemperature(testTemp)) {
            tempReadSuccess = true;
        } else {
            Serial.print("Ошибка чтения температуры HTU21D при валидации (попытка ");
            Serial.print(attempt + 1);
            Serial.println(")");
            // Используем millis() для неблокирующей задержки
            unsigned long startTime = millis();
            while (millis() - startTime < 10) {
                // Неблокирующая задержка
                delay(1);  // Небольшая задержка для предотвращения чрезмерного потребления CPU
            }
        }
    }
    
    // Попытка чтения влажности (до 3 попыток)
    for (int attempt = 0; attempt < 3 && !humidityReadSuccess; attempt++) {
        if (readHTU21DHumidity(testHumidity)) {
            humidityReadSuccess = true;
        } else {
            Serial.print("Ошибка чтения влажности HTU21D при валидации (попытка ");
            Serial.print(attempt + 1);
            Serial.println(")");
            // Используем millis() для неблокирующей задержки
            unsigned long startTime = millis();
            while (millis() - startTime < 10) {
                // Неблокирующая задержка
                delay(1);  // Небольшая задержка для предотвращения чрезмерного потребления CPU
            }
        }
    }
    
    if (!tempReadSuccess) {
        Serial.println("Не удалось прочитать температуру HTU21D после 3 попыток!");
        return false;
    }
    
    if (!humidityReadSuccess) {
        Serial.println("Не удалось прочитать влажность HTU21D после 3 попыток!");
        return false;
    }
    
    // Проверяем разумность значений
    if (testTemp < -40.0 || testTemp > 125.0) {
        Serial.println("Недопустимое значение температуры HTU21D при валидации!");
        Serial.print("Полученное значение: ");
        Serial.println(testTemp);
        return false;
    }
    
    if (testHumidity < 0.0 || testHumidity > 100.0) {
        Serial.println("Недопустимое значение влажности HTU21D при валидации!");
        Serial.print("Полученное значение: ");
        Serial.println(testHumidity);
        return false;
    }
    
    Serial.println("HTU21D прошел валидацию!");
    return true;
}

/**
 * @brief Чтение регистра HTU21D
 *
 * Вспомогательная функция для чтения регистра датчика HTU21D.
 *
 * @param reg Адрес регистра для чтения
 * @param value Переменная для хранения значения регистра
 * @return true в случае успешного чтения, false в случае ошибки
 */
bool SensorManager::readHTU21DRegister(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(htu21dAddress);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(htu21dAddress, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    value = Wire.read();
    return true;
}

/**
 * @brief Запись в регистр HTU21D
 *
 * Вспомогательная функция для записи в регистр датчика HTU21D.
 *
 * @param reg Адрес регистра для записи
 * @param value Значение для записи
 * @return true в случае успешной записи, false в случае ошибки
 */
bool SensorManager::writeHTU21DRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(htu21dAddress);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

/**
 * @brief Проверка CRC для данных HTU21D
 *
 * Выполняет проверку CRC для полученных данных HTU21D.
 *
 * @param data Двухбайтовые данные для проверки
 * @param crc CRC-байт для проверки
 * @return true если CRC корректна, false в случае ошибки
 */
bool SensorManager::checkCRC(uint16_t data, uint8_t crc) {
    // CRC-8 с полиномом 0x31 (x^8 + x^5 + x^4 + 1) для HTU21D
    // Реализация алгоритма проверки CRC-8 для HTU21D
    uint16_t remainder = data << 8;
    remainder |= crc;
    
    // Полином 0x31 (00110001) для CRC-8
    uint16_t divisor = 0x9880; // CRC polynomial (0x31) << 11
    
    for (int i = 0; i < 16; i++) {
        if (remainder & 0x8000) {
            remainder = (remainder << 1) ^ divisor;
        } else {
            remainder = (remainder << 1);
        }
    }
    
    return (remainder == 0);
}
