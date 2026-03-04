#include "Controller.h"
#include "DisplayUI.h"

Controller::Controller(SensorManager* s, RelayManager* r, TimeManager* t) 
    : sensors(s), relays(r), rtc(t), ui(nullptr) {
    currentState = SystemState::IDLE;
    currentError = ErrorCode::NONE;

    // Загрузка данных из EEPROM
    PersistentData data;
    storage.load(data);

    targetTemp = data.targetTemp;
    targetRh = data.targetRh;
    calib = data.calibration;
    stats = data.stats;

    sensors->setCalibration(calib);

    lastStatsUpdate = millis();
    lastEEPROMSave = millis();
    needsPersistentSave = false;
    lastPersistentChangeTime = 0;
    ozoneInhibitedToday = false;
    stateTimer = 0;
    manualTimer = 0;
    retryOzoneTimer = 0;
}

void Controller::init() {
    changeState(SystemState::AUTO_CLIMATE);
}

void Controller::tick() {
    unsigned long now = millis();

    // 0. Обновление статистики (раз в минуту)
    if (now - lastStatsUpdate >= 60000UL) {
        lastStatsUpdate = now;
        stats.uptimeMinutes++;
        if (relays->getFanState()) stats.fanMinutes++;
        if (relays->getOzoneState()) stats.ozoneMinutes++;
    }

    // 0.1 Сохранение статистики в EEPROM (раз в 30 минут)
    if (now - lastEEPROMSave >= 1800000UL) {
        lastEEPROMSave = now;
        needsPersistentSave = true;
        lastPersistentChangeTime = now - DEFERRED_SAVE_DELAY; // Принудительное сохранение без задержки
    }

    // 0.2 Отложенное сохранение в EEPROM
    if (needsPersistentSave && (now - lastPersistentChangeTime >= DEFERRED_SAVE_DELAY)) {
        needsPersistentSave = false;
        PersistentData data = { targetTemp, targetRh, calib, stats, 0 };
        storage.save(data);
    }

    // 1. Постоянная проверка критических ошибок
    checkCriticalErrors();

    // 2. Если есть критическая ошибка — принудительный переход в ERROR_STATE
    if (currentError != ErrorCode::NONE && currentState != SystemState::ERROR_STATE) {
        changeState(SystemState::ERROR_STATE);
    }

    // 3. Обработка состояний (FSM)
    switch (currentState) {
        case SystemState::AUTO_CLIMATE:
            handleAutoClimate();
            
            // Проверка запуска озонирования по расписанию
            if (rtc->isOzoneTimeScheduled()) {
                changeState(SystemState::OZONE_START);
            }
            // Проверка повторной попытки через 30 минут (если был запрет)
            else if (retryOzoneTimer != 0 && (millis() - retryOzoneTimer >= OZONE_RETRY_SHORT)) {
                retryOzoneTimer = 0; // Сбрасываем таймер
                changeState(SystemState::OZONE_START);
            }
            break;

        case SystemState::OZONE_START:
            handleOzoneCycle(); // Проверка условий запуска (T_out и подсветка)
            break;

        case SystemState::OZONE_ACTIVE:
        case SystemState::OZONE_HOLD:
        case SystemState::OZONE_VENT:
            handleOzoneCycle(); // Логика фаз
            break;

        case SystemState::MANUAL_FAN:
        case SystemState::MANUAL_OZONE:
            handleManualModes();
            break;

        case SystemState::ERROR_STATE:
            relays->setFan(false, true);   // Выключить всё немедленно
            relays->setOzone(false);
            break;

        default:
            break;
    }
}

void Controller::handleAutoClimate() {
    SensorData in = sensors->getInside();
    SensorData out = sensors->getOutside();

    // Алгоритм (Вариант С) из ТЗ
    bool needsAction = (in.temp > (targetTemp + HYSTERESIS_TEMP)) ||
                       (in.rh > (targetRh + HYSTERESIS_RH));
    
    bool airIsBetter = (out.ah + MARGIN_AH) < in.ah;
    
    bool condensationSafe = (in.dewpoint + MARGIN_COND_SAFETY) < in.temp;

    // Условие включения вентилятора
    if (needsAction && airIsBetter && condensationSafe) {
        relays->setFan(true); 
    } else {
        relays->setFan(false);
    }
}

void Controller::handleOzoneCycle() {
    SensorData out = sensors->getOutside();
    
    // Проверка условий запрета (только для автоматического старта)
    if (currentState == SystemState::OZONE_START) {
        bool tempInhibited = (out.temp < 0.0f);
        bool uiInhibited = (ui != nullptr && ui->isBacklightOn());

        if (tempInhibited || uiInhibited) {
            #ifdef DEBUG
            Serial.println(F("Ozone Inhibited: Wait 30m"));
            #endif
            retryOzoneTimer = millis();
            // rtc->resetOzoneTrigger() не требуется, так как повторный вход
            // через 30 минут управляется таймером retryOzoneTimer в tick()
            changeState(SystemState::AUTO_CLIMATE);
            return;
        }
        
        // Если всё ок — включаем озон
        relays->setFan(false, true);
        relays->setOzone(true);
        stateTimer = millis();
        changeState(SystemState::OZONE_ACTIVE);
    }

    // Фаза 1: Озонирование (15 мин)
    if (currentState == SystemState::OZONE_ACTIVE) {
        if (millis() - stateTimer >= OZONE_WORK_TIME) {
            relays->setOzone(false);
            stateTimer = millis();
            changeState(SystemState::OZONE_HOLD);
        }
    }

    // Фаза 2: Пауза (2 часа)
    if (currentState == SystemState::OZONE_HOLD) {
        if (millis() - stateTimer >= OZONE_HOLD_TIME) {
            stateTimer = millis();
            changeState(SystemState::OZONE_VENT);
        }
    }

    // Фаза 3: Проветривание (15 мин)
    if (currentState == SystemState::OZONE_VENT) {
        // Проветривание разрешено только если на улице не мороз
        if (out.temp > 0.0f) {
            relays->setFan(true);
            if (millis() - stateTimer >= OZONE_VENT_TIME) {
                relays->setFan(false);
                changeState(SystemState::AUTO_CLIMATE);
            }
        } else {
            // Если мороз — принудительно выходим
            relays->setFan(false);
            changeState(SystemState::AUTO_CLIMATE);
        }
    }
}

void Controller::handleManualModes() {
    if (millis() - stateTimer >= (manualTimer * 60000UL)) {
        relays->setFan(false);
        relays->setOzone(false);
        changeState(SystemState::AUTO_CLIMATE);
    }
}

void Controller::checkCriticalErrors() {
    // 1. Ошибки от SensorManager (Hardware + Mismatch)
    ErrorCode sErr = sensors->checkErrors();
    if (sErr != ErrorCode::NONE) {
        currentError = sErr;
        return;
    }

    // 2. Ошибки от TimeManager
    ErrorCode tErr = rtc->checkErrors();
    if (tErr != ErrorCode::NONE) {
        currentError = tErr;
        return;
    }

    // 3. Проверка параметров климата
    SensorData in = sensors->getInside();
    if (in.temp <= TEMP_CRITICAL_MIN) {
        currentError = ErrorCode::TEMP_TOO_LOW;
    }
    else if (in.dewpoint >= (in.temp - CONDENSATION_ERR_DIFF)) {
        currentError = ErrorCode::CONDENSATION_RISK;
    }
}

void Controller::changeState(SystemState newState) {
    #ifdef DEBUG
    Serial.print(F("FSM: ")); Serial.print((int)currentState);
    Serial.print(F(" -> ")); Serial.println((int)newState);
    #endif
    currentState = newState;
}

void Controller::resetError() {
    // Ручной сброс ошибки возможен только если физическая причина устранена
    currentError = ErrorCode::NONE;
    changeState(SystemState::IDLE);
    init(); // Пробуем запуститься снова
}

void Controller::startManualFan(uint16_t minutes) {
    if (currentState == SystemState::ERROR_STATE) return;
    manualTimer = minutes;
    stateTimer = millis();
    relays->setFan(true, true);
    changeState(SystemState::MANUAL_FAN);
}

void Controller::startManualOzone(uint16_t minutes) {
    if (currentState == SystemState::ERROR_STATE) return;
    manualTimer = minutes;
    stateTimer = millis();
    relays->setOzone(true);
    changeState(SystemState::MANUAL_OZONE);
}

void Controller::setTargetTemp(float t) {
    targetTemp = t;
    needsPersistentSave = true;
    lastPersistentChangeTime = millis();
}

void Controller::setTargetRh(float h) {
    targetRh = h;
    needsPersistentSave = true;
    lastPersistentChangeTime = millis();
}

void Controller::setCalibration(const CalibrationData& data) {
    calib = data;
    sensors->setCalibration(calib);
    needsPersistentSave = true;
    lastPersistentChangeTime = millis();
}

void Controller::resetStats() {
    stats = {0, 0, 0};
    needsPersistentSave = true;
    lastPersistentChangeTime = millis();
}
