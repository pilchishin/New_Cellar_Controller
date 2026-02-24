#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "Config.h" // Предполагается, что здесь заданы пины PIN_RELAY_FAN и PIN_RELAY_OZONE

class RelayManager {
private:
    // Текущие логические состояния реле
    bool fanState;
    bool ozoneState;

    // Временная метка последнего переключения вентилятора (для антидребезга)
    unsigned long lastFanChangeTime;

    // Константа: 5 минут в миллисекундах (5 * 60 * 1000)
    // Символы UL (Unsigned Long) обязательны, чтобы компилятор не переполнил 16-битный int
    const unsigned long FAN_DEBOUNCE_DELAY = 300000UL; 

public:
    // Конструктор
    RelayManager();

    // Инициализация пинов (настройка OUTPUT и начального состояния)
    void init();

    // Установка состояния вентилятора. 
    // Параметр state: true - включить, false - выключить
    // Параметр force: если true, игнорирует 5-минутный таймер (для аварийного отключения)
    void setFan(bool state, bool force = false);

    // Установка состояния озонатора. 
    // Озонатор работает строго по таймерам FSM (15 мин), ему аппаратный антидребезг не нужен
    void setOzone(bool state);

    // Геттеры для отображения статусов на экране LCD (в меню Status)
    bool getFanState() const { return fanState; }
    bool getOzoneState() const { return ozoneState; }
};

#endif