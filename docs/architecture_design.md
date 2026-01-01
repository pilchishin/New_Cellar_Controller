# Архитектурный дизайн контроллера вентиляции и озонирования подвала

## Общая архитектура

Система построена по модульной архитектуре с использованием объектно-ориентированного подхода. Каждый модуль отвечает за свою функциональную область и взаимодействует с другими модулями через строго определенные интерфейсы.

## Структура проекта

```
src/
├── main.cpp              // Основной файл с loop() и setup()
├── config.h              // Конфигурационные определения
├── sensors.h/cpp         // Классы датчиков и их обработка
├── filters.h/cpp         // Фильтры для сенсорных данных
├── climate.h/cpp         // Логика климат-контроля
├── ozone.h/cpp           // Логика озонирования
├── fsm.h/cpp             // Машина состояний
├── eeprom.h/cpp          // Работа с энергонезависимой памятью
├── menu.h/cpp            // Меню управления
├── display.h/cpp         // Управление дисплеем
├── rtc.h/cpp             // Работа с RTC DS3231
└── utils.h/cpp           // Вспомогательные функции
```

## Класс SensorManager

Класс для управления всеми датчиками системы.

### Поля:
- `bme280` - датчик BME280 (внутри)
- `ds18b20` - датчик DS18B20 (контрольный датчик подвала)
- `htu21d` - датчик HTU21D (улица)
- `sensorData` - структура с текущими данными датчиков
- `lastReadTime` - время последнего опроса датчиков
- `sensorOffsets` - смещения датчиков из EEPROM

### Методы:
- `initialize()` - инициализация всех датчиков
- `readSensors()` - чтение всех датчиков с фильтрацией
- `checkCriticalErrors()` - проверка критических ошибок датчиков
- `applyOffsets()` - применение калибровочных смещений
- `getFilteredData()` - получение отфильтрованных данных

## Класс Filter

Класс для фильтрации данных датчиков.

### Поля:
- `medianBuffer` - буфер для медианного фильтра (N=3)
- `emaValue` - текущее значение EMA (α=0.2)
- `filterType` - тип фильтра (медианный или EMA)

### Методы:
- `addValue(value)` - добавление нового значения в фильтр
- `getFilteredValue()` - получение отфильтрованного значения
- `reset()` - сброс фильтра

## Класс ClimateController

Класс для реализации алгоритма климат-контроля.

### Поля:
- `targetTemp` - целевая температура (4°C)
- `targetRH` - целевая влажность (85%)
- `tempHysteresis` - гистерезис температуры (±0.5°C)
- `rhHysteresis` - гистерезис влажности (±3%)
- `condensationMargin` - безопасный запас от конденсации (2°C)
- `minTemp` - минимальная температура (+2°C)
- `lastFanToggle` - время последнего переключения вентилятора
- `fanState` - текущее состояние вентилятора
- `fanCooldownActive` - активен ли антидребезг вентилятора

### Методы:
- `calculateAbsoluteHumidity(temp, rh)` - расчет абсолютной влажности
- `calculateDewPoint(temp, rh)` - расчет точки росы
- `shouldActivateFan()` - проверка условий включения вентилятора
- `updateFanState()` - обновление состояния вентилятора
- `checkSafetyConditions()` - проверка безопасных условий
- `setTargets(temp, rh)` - установка целевых значений

## Класс OzoneController

Класс для управления циклом озонирования.

### Поля:
- `scheduleEnabled` - включено ли расписание
- `ozoneWeekday` - день недели для озонирования
- `ozoneTime` - время озонирования (HH:MM)
- `ozoneDuration` - продолжительность озонирования (15 мин)
- `holdDuration` - продолжительность паузы (2 часа)
- `ventDuration` - продолжительность проветривания (15 мин)
- `cycleStartTime` - время начала цикла
- `currentPhase` - текущая фаза цикла
- `ozoneState` - состояние озонатора
- `manualMode` - ручной режим озонирования

### Методы:
- `checkSchedule()` - проверка необходимости запуска по расписанию
- `startOzoneCycle()` - запуск цикла озонирования
- `updateCycle()` - обновление текущей фазы цикла
- `checkSafetyConditions()` - проверка условий безопасности
- `abortCycle()` - прерывание цикла
- `setSchedule(weekday, time)` - установка расписания
- `manualOzone(duration)` - ручной запуск озонирования

## Класс FSM (Finite State Machine)

Класс для реализации машины состояний системы.

### Перечисление состояний:
```cpp
enum class State {
    IDLE,
    AUTO_CLIMATE_CONTROL,
    OZONE_START,
    OZONE_ACTIVE,
    OZONE_HOLD,
    OZONE_VENT,
    OZONE_ABORT,
    MANUAL_FAN,
    MANUAL_OZONE,
    ERROR
};
```

### Поля:
- `currentState` - текущее состояние
- `previousState` - предыдущее состояние
- `stateStartTime` - время начала текущего состояния
- `climateController` - ссылка на климат-контроллер
- `ozoneController` - ссылка на озон-контроллер
- `sensorManager` - ссылка на менеджер датчиков

### Методы:
- `transitionTo(newState)` - переход в новое состояние
- `update()` - обновление машины состояний
- `handleError()` - обработка ошибок
- `canTransitionTo(newState)` - проверка возможности перехода

## Класс EEPROMManager

Класс для работы с энергонезависимой памятью.

### Поля:
- `eepromMap` - структура сопоставления адресов EEPROM
- `crc` - контрольная сумма

### Методы:
- `readConfig()` - чтение конфигурации из EEPROM
- `writeConfig()` - запись конфигурации в EEPROM
- `readTargetTemp()` - чтение целевой температуры
- `writeTargetTemp(temp)` - запись целевой температуры
- `readTargetRH()` - чтение целевой влажности
- `writeTargetRH(rh)` - запись целевой влажности
- `readSensorOffsets()` - чтение смещений датчиков
- `writeSensorOffsets()` - запись смещений датчиков
- `readSchedule()` - чтение расписания озонирования
- `writeSchedule()` - запись расписания озонирования
- `calculateCRC()` - расчет контрольной суммы

## Класс MenuManager

Класс для управления меню.

### Поля:
- `display` - ссылка на дисплей
- `currentMenu` - текущий уровень меню
- `menuItems` - элементы меню
- `selectedItem` - выбранный элемент меню
- `menuDepth` - глубина меню

### Методы:
- `displayMenu()` - отображение текущего меню
- `navigateUp()` - навигация вверх
- `navigateDown()` - навигация вниз
- `selectItem()` - выбор элемента
- `handleInput(button)` - обработка нажатий кнопок
- `enterMenu(menuLevel)` - вход в уровень меню
- `exitMenu()` - выход из меню

## Класс DisplayManager

Класс для управления дисплеем LCD 1602.

### Поля:
- `lcd` - объект дисплея
- `backlightEnabled` - состояние подсветки
- `displayMode` - текущий режим отображения
- `lastUpdate` - время последнего обновления

### Методы:
- `initialize()` - инициализация дисплея
- `displayStatus()` - отображение статуса системы
- `displayMenu()` - отображение меню
- `displayError()` - отображение ошибки
- `enableBacklight()` - включение подсветки
- `disableBacklight()` - выключение подсветки
- `updateDisplay()` - обновление дисплея
- `clear()` - очистка дисплея

## Класс RTCManager

Класс для работы с RTC DS3231.

### Поля:
- `rtc` - объект RTC
- `lastSyncTime` - время последней синхронизации
- `timeValid` - действительность времени

### Методы:
- `initialize()` - инициализация RTC
- `readTime()` - чтение текущего времени
- `setTime(time)` - установка времени
- `checkTimeAccuracy()` - проверка точности времени
- `scheduleAlarm()` - настройка будильника
- `isTimeValid()` - проверка действительности времени

## Класс Utils

Класс для вспомогательных функций.

### Методы:
- `calculateAbsoluteHumidity(temp, rh)` - расчет абсолютной влажности
- `calculateDewPoint(temp, rh)` - расчет точки росы
- `checkCondensationRisk(temp, dewPoint)` - проверка риска конденсации
- `mapValue(value, fromMin, fromMax, toMin, toMax)` - преобразование значения
- `isTimeInRange(current, start, end)` - проверка вхождения во временной диапазон
- `formatTime(time)` - форматирование времени
- `formatDate(date)` - форматирование даты

## Взаимодействие модулей

```
Main Loop
├── SensorManager: readSensors()
├── FSM: update()
│   ├── ClimateController: updateFanState()
│   ├── OzoneController: updateCycle()
│   └── DisplayManager: updateDisplay()
├── MenuManager: handleInput()
└── EEPROMManager: syncIfNeeded()
```

## Использование периферии

- **BME280**: I2C, внутренние показания (T, RH)
- **DS18B20**: 1-Wire, контрольный датчик подвала (T)
- **HTU21D**: I2C, внешние показания (T, RH)
- **DS3231**: I2C, реальное время
- **LCD 1602**: I2C, отображение информации
- **Кнопки**: цифровые входы с подтяжкой
- **SSR реле**: цифровые выходы для вентилятора и озонатора

## Алгоритм климат-контроля

```
if ( (T_in > TargetTemp+hyst OR RH_in > TargetRH+hyst)
 AND (AH_out + margin < AH_in)
  AND (dewpoint_in + 2°C < T_in) ) then FAN ON
else FAN OFF
```

## Алгоритм озонирования

1. Проверка расписания через DS3231
2. Проверка условий безопасности (T_out > 0°C, отсутствие подсветки)
3. Фазы цикла:
   - OZONE_ACTIVE: 15 минут
   - OZONE_HOLD: 2 часа
   - OZONE_VENT: 15 минут форсированного проветривания

## Обработка ошибок

- **Критические ошибки**: требуют ручного сброса
  - Несовпадение BME280 и DS18B20 > 2°C
  - Ошибки датчиков
  - Риск конденсации
  - T_in ≤ +2°C
  - Ошибки DS3231 (3 попытки чтения)

- **Мягкие ошибки**: автоматическое восстановление

## Конфигурация

Все настройки хранятся в EEPROM согласно заданной карте:

| Адрес | Размер | Назначение |
|-------|--------|------------|
| 0x00 | 2 байт | Target T (x10) |
| 0x02 | 2 байт | Target RH (x10) |
| 0x04 | 2 байт | BME280 T offset (x10) |
| 0x06 | 2 байт | BME280 RH offset (x10) |
| 0x08 | 2 байт | DS18B20 T offset (x10) |
| 0x0A | 2 байт | HTU21D T offset (x10) |
| 0x0C | 2 байт | HTU21D RH offset (x10) |
| 0x0E | 1 байт | OZONE schedule enable |
| 0x0F | 1 байт | OZONE weekday |
| 0x10 | 2 байт | OZONE hour/min HHMM |
| 0x12 | 2 байт | DS3231 calibration (ppm, signed) |
| 0x14 | 32 байт | Wear leveling / config copy |
| 0x34 | 64 байт | Counter snapshot FAN/OZONE hh:mm |
| 0x74 | 128 байт | Event log (20 записей x 6 байт) |
| ... | ... | CRC / контрольная сумма |

## Отладка

Отладочный вывод через Serial при определении `DEBUG` в config.h