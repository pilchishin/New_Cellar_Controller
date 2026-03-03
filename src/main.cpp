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

    // 4. Включаем Watchdog таймер на 8 секунд
    // Если код зависнет, Arduino автоматически перезагрузится
    wdt_enable(WDT_TIMEOUT);
    
    #ifdef DEBUG
    Serial.println(F("System Ready. Watchdog Enabled."));
    #endif
}

void loop() {
    // Сбрасываем таймер Watchdog в начале каждого цикла
    // "Кормим собаку", чтобы она не перезагрузила контроллер
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