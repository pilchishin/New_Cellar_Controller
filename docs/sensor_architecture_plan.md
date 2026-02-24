# Архитектурный план: Система опроса датчиков BME280, HTU21D и DS18B20

## Общее описание

Разработать архитектуру кода для опроса датчиков BME280, HTU21D и DS18B20 на Arduino, чтобы каждый датчик опрашивался отдельно и возвращал данные через функции. Архитектура должна быть неблокирующей и позволять опрашивать каждый датчик отдельно.

## Требования

- Arduino nano
- Язык: C++ (Arduino IDE)
- Использовать библиотеки: 
  - HTU21D → Adafruit HTU21DF
  - BME280 → Adafruit BME280
  - DS18B20 → OneWire + DallasTemperature
- Отдельно получать: температуру и влажность с BME280, температуру и влажность с HTU21D, температуру с DS18B20
- Оформить в виде функций: `getBmeTemp()`, `getBmeHum()`, `getHtuTemp()`, `getHtuHum()`, `getDsTemp()`
- Без `delay()`, с классами
- Архитектура должна позволять опрашивать каждый датчик отдельно
- Архитектура должна быть эффективной и не блокировать выполнение других задач

## Архитектурное решение

### Структура класса SensorManager

class SensorManager {
private:
    // Флаги инициализации датчиков
    bool bme280Initialized;
    bool htu21dInitialized;
    bool ds18b20Initialized;
    
    // Адреса датчиков
    unsigned char bme280Address;
    unsigned char htu21dAddress;
    DeviceAddress ds18b20Address;
    
    // Данные датчиков
    float bme280Temperature;
    float bme280Humidity;
    float htu21dTemperature;
    float htu21dHumidity;
    float ds18b20Temperature;
    
    // Флаги валидности данных
    bool bme280TempValid;
    bool bme280HumidityValid;
    bool htu21dTempValid;
    bool htu21dHumidityValid;
    bool ds18b20TempValid;
    
    // Временные метки для неблокирующих операций
    unsigned long lastBmeReadTime;
    unsigned long lastHtuReadTime;
    unsigned long lastDsReadTime;
    unsigned long htu21dStartTime;
    unsigned long ds18b20StartTime;
    
    // Состояния для неблокирующих операций
    enum Htu21dState { HTU21D_IDLE, HTU21D_MEASURING_TEMP, HTU21D_MEASURING_HUM, HTU21D_READING_TEMP, HTU21D_READING_HUM };
    enum Ds18b20State { DS18B20_IDLE, DS18B20_REQUESTING, DS18B20_READING };
    
    Htu21dState htu21dState;
    Ds18b20State ds18b20State;
    
    // Методы для неблокирующего чтения
    bool readBme280();
    bool readHtu21d();
    bool readDs18b20();
    
public:
    SensorManager();
    void init();
    bool update();
    
    // Функции получения данных
    float getBmeTemp();
    float getBmeHum();
    float getHtuTemp();
    float getHtuHum();
    float getDsTemp();
    
    // Функции проверки валидности
    bool isBmeTempValid();
    bool isBmeHumValid();
    bool isHtuTempValid();
    bool isHtuHumValid();
    bool isDsTempValid();
};
```

### Основные методы

#### Конструктор
- Инициализирует все внутренние переменные и флаги
- Устанавливает начальные значения состояний

#### Метод init()
- Инициализирует все датчики (BME280, HTU21D, DS18B20)
- Проверяет наличие датчиков
- Устанавливает флаги инициализации

#### Метод update()
- Основной метод для неблокирующего опроса датчиков
- Управляет состояниями HTU21D и DS18B20
- Вызывает методы чтения для каждого датчика
- Возвращает true если обновление выполнено, false если ожидается таймер

#### Функции получения данных
- `getBmeTemp()` - возвращает температуру с BME280
- `getBmeHum()` - возвращает влажность с BME280
- `getHtuTemp()` - возвращает температуру с HTU21D
- `getHtuHum()` - возвращает влажность с HTU21D
- `getDsTemp()` - возвращает температуру с DS18B20

### Неблокирующая архитектура

#### HTU21D
Использует конечный автомат с состояниями:
- `HTU21D_IDLE` - ожидание
- `HTU21D_MEASURING_TEMP` - отправлена команда измерения температуры
- `HTU21D_MEASURING_HUM` - отправлена команда измерения влажности
- `HTU21D_READING_TEMP` - ожидание завершения измерения температуры
- `HTU21D_READING_HUM` - ожидание завершения измерения влажности

#### DS18B20
Использует конечный автомат с состояниями:
- `DS18B20_IDLE` - ожидание
- `DS18B20_REQUESTING` - отправлен запрос на измерение
- `DS18B20_READING` - ожидание завершения измерения

#### BME280
Для BME280 чтение происходит быстрее, поэтому можно читать данные напрямую без конечного автомата (если это допустимо по спецификации датчика)

### Схема работы

```mermaid
graph TD
    A[update()] --> B{BME280 инициализирован?}
    B -->|Yes| C[readBme280()]
    B -->|No| D{HTU21D инициализирован?}
    C --> D
    D -->|Yes| E[Проверить состояние HTU21D]
    D -->|No| F{DS18B20 инициализирован?}
    E --> G{HTU21D_IDLE?}
    G -->|Yes| H[Запустить измерение HTU21D]
    G -->|No| I[Обработать текущее состояние HTU21D]
    H --> F
    I --> F
    F -->|Yes| J[Проверить состояние DS18B20]
    F -->|No| K[Вернуть результат]
    J --> L{DS18B20_IDLE?}
    L -->|Yes| M[Запросить измерение DS18B20]
    L -->|No| N[Обработать текущее состояние DS18B20]
    M --> K
    N --> K
    K --> O[Вернуть true]
```

### Тайминги

- HTU21D измерение температуры: до 50 мс
- HTU21D измерение влажности: до 16 мс
- DS18B20 измерение температуры: до 750 мс при 12-битном разрешении

### Пример использования

```cpp
SensorManager sensorManager;

void setup() {
 Serial.begin(9600);
  sensorManager.init();
}

void loop() {
 sensorManager.update(); // Вызывать регулярно для неблокирующего опроса
  
  // Получение данных от датчиков
  if (sensorManager.isBmeTempValid()) {
    float bmeTemp = sensorManager.getBmeTemp();
    Serial.print("BME280 Temperature: ");
    Serial.println(bmeTemp);
  }
  
  if (sensorManager.isHtuTempValid()) {
    float htuTemp = sensorManager.getHtuTemp();
    Serial.print("HTU21D Temperature: ");
    Serial.println(htuTemp);
  }
  
  if (sensorManager.isDsTempValid()) {
    float dsTemp = sensorManager.getDsTemp();
    Serial.print("DS18B20 Temperature: ");
    Serial.println(dsTemp);
  }
  
  delay(100); // Другие задачи могут выполняться в этом месте
}
```

## Преимущества архитектуры

1. **Неблокирующая работа** - основной цикл программы не блокируется во время опроса датчиков
2. **Отдельный опрос каждого датчика** - можно читать данные с каждого датчика независимо
3. **Эффективное использование времени** - между операциями чтения можно выполнять другие задачи
4. **Гибкость** - можно настроить частоту опроса каждого датчика отдельно
5. **Обработка ошибок** - флаги валидности позволяют проверять корректность данных