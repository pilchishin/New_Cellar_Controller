#include <Arduino.h>
#include <avr/wdt.h> // Библиотека для работы со сторожевым таймером

// Подключаем наши модули
#include "Config.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "TimeManager.h"
#include "Controller.h"
#include "DisplayUI.h"

// Создаем экземпляры модулей (выделение памяти при старте)
SensorManager sensorManager;
RelayManager  relayManager;
TimeManager   timeManager;

// Контроллер объединяет логику датчиков, реле и времени
Controller controller(&sensorManager, &relayManager, &timeManager);

// Интерфейс связывает контроллер и пользователя
DisplayUI ui(&controller, &sensorManager, &timeManager);

// Переменные для управления интервалами опроса
unsigned long lastSensorUpdate = 0;
unsigned long lastTimeUpdate   = 0;

void setup() {
    // 1. Инициализация отладки
    #ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
    Serial.println(F("--- CELLAR CONTROL SYSTEM STARTING ---"));
    #endif

    // 2. Инициализация аппаратных модулей
    relayManager.init();   // Реле — в первую очередь (безопасное состояние)
    sensorManager.init();  // Датчики
    timeManager.init();    // Часы реального времени
    ui.init();             // Дисплей и кнопки

    // 3. Связываем контроллер с интерфейсом
    // Это нужно, чтобы контроллер мог знать, включена ли подсветка (условие озонирования)
    controller.setUI(&ui);
    controller.init();

    // 4. Настройка сторожевого таймера (Watchdog) на 8 секунд.
    // Если программа зависнет в цикле (например, при сбое I2C),
    // через 8 секунд контроллер будет принудительно перезагружен.
    wdt_enable(WDT_TIMEOUT);
    
    #ifdef DEBUG
    Serial.println(F("System Ready. Watchdog Enabled."));
    #endif
}

/**
 * @brief Главный цикл программы (Loop).
 * Выполняется непрерывно. Здесь сбрасывается Watchdog и распределяются задачи
 * по времени (опрос датчиков, обновление UI, работа FSM).
 */
void loop() {
    // "Кормление собаки" — сброс таймера Watchdog для предотвращения ребута
    wdt_reset();

    unsigned long currentMillis = millis();

    // 1. Опрос датчиков по интервалу (раз в 10 секунд по ТЗ)
    if (currentMillis - lastSensorUpdate >= SENSOR_POLL_INTERVAL) {
        lastSensorUpdate = currentMillis;
        sensorManager.update();
        
        #ifdef DEBUG
        Serial.println(F("Sensors updated."));
        #endif
    }

    // 2. Обновление времени (раз в 1 секунду достаточно для расписания)
    if (currentMillis - lastTimeUpdate >= 1000) {
        lastTimeUpdate = currentMillis;
        timeManager.update();
    }

    // 3. Основной такт логики (FSM)
    // Выполняется максимально часто для мгновенной реакции на ошибки
    controller.tick();

    // 4. Обновление интерфейса (обработка кнопок и отрисовка LCD)
    // Внутри DisplayUI.update уже есть свои таймеры для отрисовки
    ui.update();
}