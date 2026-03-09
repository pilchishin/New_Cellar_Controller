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
SensorManager sensor_manager;
RelayManager relay_manager;
TimeManager time_manager;

// Контроллер объединяет логику датчиков, реле и времени
Controller controller(&sensor_manager, &relay_manager, &time_manager);

// Интерфейс связывает контроллер и пользователя
DisplayUI ui(&controller, &sensor_manager, &time_manager);

// Переменные для управления интервалами опроса
unsigned long last_sensor_update = 0;
unsigned long last_time_update = 0;

void setup() {
  // 1. Инициализация отладки
#ifdef DEBUG
  Serial.begin(SERIAL_SPEED);
  Serial.println(F("--- CELLAR CONTROL SYSTEM STARTING ---"));
#endif

  // 2. Инициализация аппаратных модулей
  relay_manager.Init();   // Реле — в первую очередь (безопасное состояние)
  sensor_manager.Init();  // Датчики
  time_manager.Init();    // Часы реального времени
  ui.Init();              // Дисплей и кнопки

  // 3. Связываем контроллер с интерфейсом
  // Это нужно, чтобы контроллер мог знать, включена ли подсветка (условие
  // озонирования)
  controller.SetUI(&ui);
  controller.Init();

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

  unsigned long current_millis = millis();

  // 1. Опрос датчиков по интервалу (раз в 10 секунд по ТЗ)
  if (current_millis - last_sensor_update >= kSensorPollInterval) {
    last_sensor_update = current_millis;
    sensor_manager.Update();

#ifdef DEBUG
    Serial.println(F("Sensors updated."));
#endif
  }

  // 2. Обновление времени (раз в 1 секунду достаточно для расписания)
  if (current_millis - last_time_update >= 1000) {
    last_time_update = current_millis;
    time_manager.Update();
  }

  // 3. Основной такт логики (FSM)
  // Выполняется максимально часто для мгновенной реакции на ошибки
  controller.Tick();

  // 4. Обновление интерфейса (обработка кнопок и отрисовка LCD)
  // Внутри DisplayUI.update уже есть свои таймеры для отрисовки
  ui.Update();
}