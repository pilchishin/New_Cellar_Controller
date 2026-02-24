#include "RelayManager.h"

RelayManager::RelayManager() {
    fanState = false;
    ozoneState = false;
    
    // Инициализируем нулем. Это создаст полезный побочный эффект: 
    // при включении питания контроллера вентилятор не сможет включиться первые 5 минут.
    // Это даст датчикам время на прогрев и стабилизацию показаний (особенно BME280).
    lastFanChangeTime = 0; 
}

void RelayManager::init() {
    #ifdef DEBUG
    Serial.println(F("Init Relays..."));
    #endif

    // Настраиваем пины управления как выходы
    pinMode(PIN_RELAY_FAN, OUTPUT);
    pinMode(PIN_RELAY_OZONE, OUTPUT);

    // Устанавливаем начальное безопасное состояние — ВСЕ ВЫКЛЮЧЕНО.
    // ПРИМЕЧАНИЕ: Твердотельные реле (SSR) обычно включаются высоким уровнем (HIGH).
    // Если используете инверсные модули реле, замените LOW на HIGH здесь и в методах ниже.
    digitalWrite(PIN_RELAY_FAN, LOW);
    digitalWrite(PIN_RELAY_OZONE, LOW);
}

void RelayManager::setFan(bool requestedState, bool force) {
    // Если запрашиваемое состояние уже равно текущему, ничего не делаем
    if (this->fanState == requestedState) {
        return;
    }

    unsigned long currentTime = millis();

    // Логика антидребезга:
    // Проверяем, прошло ли 5 минут с момента последнего изменения состояния.
    // Если установлен флаг force (например, при критической ошибке), игнорируем таймер.
    if (force || (currentTime - lastFanChangeTime >= FAN_DEBOUNCE_DELAY)) {
        
        // Обновляем текущее состояние и сбрасываем таймер
        this->fanState = requestedState;
        this->lastFanChangeTime = currentTime;
        
        // Подаем сигнал на физический пин
        digitalWrite(PIN_RELAY_FAN, requestedState ? HIGH : LOW);
        
        #ifdef DEBUG
        Serial.print(F("FAN Relay changed to: "));
        Serial.println(requestedState ? F("ON") : F("OFF"));
        if (force) Serial.println(F(" (FORCED)"));
        #endif
    } else {
        // Если 5 минут не прошло, команда игнорируется аппаратно.
        // FSM из Controller.cpp будет продолжать присылать команду setFan(true/false) каждый цикл,
        // и как только 5 минут истекут, условие выполнится и реле переключится.
    }
}

void RelayManager::setOzone(bool requestedState) {
    // Защита от лишних вызовов digitalWrite
    if (this->ozoneState == requestedState) {
        return;
    }

    // Озонатор переключается мгновенно по запросу (логика времени заложена в самом FSM)
    this->ozoneState = requestedState;
    digitalWrite(PIN_RELAY_OZONE, requestedState ? HIGH : LOW);

    #ifdef DEBUG
    Serial.print(F("OZONE Relay changed to: "));
    Serial.println(requestedState ? F("ON") : F("OFF"));
    #endif
}