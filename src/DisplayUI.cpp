#include "DisplayUI.h"
#include "Config.h"

// Конструктор: адрес 0x27 и размер 16x2
DisplayUI::DisplayUI(Controller* c, SensorManager* s, TimeManager* t) 
    : lcd(0x27, 16, 2), controller(c), sensors(s), rtc(t),
      currentPage(MenuPage::STATUS_IN), lastBtnCheck(0), 
      menuBtnPressed(false), backlightOn(true) {}

void DisplayUI::init() {
    lcd.init();
    lcd.backlight();
    pinMode(BT_UP, INPUT_PULLUP);
    pinMode(BT_DOWN, INPUT_PULLUP);
    pinMode(BT_MENU, INPUT_PULLUP);
    lastActivityTime = millis();
}

void DisplayUI::update() {
    handleButtons();   // Опрос кнопок
    updateBacklight(); // Управление светом
    
    // Обновляем экран раз в 500мс, чтобы не мерцал
    static unsigned long lastDraw = 0;
    if (millis() - lastDraw >= 500) {
        lastDraw = millis();
        drawPage();
    }
}

void DisplayUI::handleButtons() {
    if (millis() - lastBtnCheck < 50) return; // Простейший антидребезг
    lastBtnCheck = millis();

    bool up = !digitalRead(BT_UP);
    bool down = !digitalRead(BT_DOWN);
    bool menu = !digitalRead(BT_MENU);

    if (up || down || menu) {
        lastActivityTime = millis(); // Сброс таймера подсветки
        if (!backlightOn) {
            lcd.backlight();
            backlightOn = true;
            return; // Первое нажатие только включает свет
        }
    }

    // Логика кнопки MENU (переключение страниц и длинное нажатие)
    if (menu) {
        if (!menuBtnPressed) {
            menuBtnPressed = true;
            menuBtnTimer = millis();
        }
    } else {
        if (menuBtnPressed) {
            unsigned long pressDuration = millis() - menuBtnTimer;
            if (pressDuration < 600) {
                // Короткое нажатие: листаем страницы вперед
                int next = (int)currentPage + 1;
                if (next > (int)MenuPage::ERROR_LOG) next = 0;
                currentPage = (MenuPage)next;
                lcd.clear();
            } else {
                // Длинное нажатие
                if (currentPage == MenuPage::STATS) {
                    controller->resetStats();
                    lcd.clear();
                    lcd.print(F("STATS RESET"));
                    delay(1000);
                }
            }
            menuBtnPressed = false;
        }
    }

    // Кнопки UP/DOWN для навигации или изменения параметров
    if (up && !menuBtnPressed) {
        CalibrationData c = controller->getCalibration();
        switch (currentPage) {
            case MenuPage::SET_TEMP:
                controller->setTargetTemp(controller->getTargetTemp() + 0.1f); break;
            case MenuPage::SET_HUM:
                controller->setTargetRh(controller->getTargetRh() + 1.0f);
                if (controller->getTargetRh() > 100.0f) controller->setTargetRh(100.0f); break;
            case MenuPage::MANUAL_MODES:
                controller->startManualFan(30); break;
            case MenuPage::CALIB_BME_T:
                c.bmeTempOffset += 0.1f; if (c.bmeTempOffset > 5.0f) c.bmeTempOffset = 5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_BME_H:
                c.bmeHumOffset += 0.1f; if (c.bmeHumOffset > 5.0f) c.bmeHumOffset = 5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_HTU_T:
                c.htuTempOffset += 0.1f; if (c.htuTempOffset > 5.0f) c.htuTempOffset = 5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_HTU_H:
                c.htuHumOffset += 0.1f; if (c.htuHumOffset > 5.0f) c.htuHumOffset = 5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_DS_T:
                c.dsTempOffset += 0.1f; if (c.dsTempOffset > 5.0f) c.dsTempOffset = 5.0f;
                controller->setCalibration(c); break;
            case MenuPage::ERROR_LOG:
                controller->resetError(); break;
            default: break;
        }
        delay(100);
    }
    if (down && !menuBtnPressed) {
        CalibrationData c = controller->getCalibration();
        switch (currentPage) {
            case MenuPage::SET_TEMP:
                controller->setTargetTemp(controller->getTargetTemp() - 0.1f); break;
            case MenuPage::SET_HUM:
                controller->setTargetRh(controller->getTargetRh() - 1.0f);
                if (controller->getTargetRh() < 0.0f) controller->setTargetRh(0.0f); break;
            case MenuPage::MANUAL_MODES:
                controller->startManualOzone(15); break;
            case MenuPage::CALIB_BME_T:
                c.bmeTempOffset -= 0.1f; if (c.bmeTempOffset < -5.0f) c.bmeTempOffset = -5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_BME_H:
                c.bmeHumOffset -= 0.1f; if (c.bmeHumOffset < -5.0f) c.bmeHumOffset = -5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_HTU_T:
                c.htuTempOffset -= 0.1f; if (c.htuTempOffset < -5.0f) c.htuTempOffset = -5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_HTU_H:
                c.htuHumOffset -= 0.1f; if (c.htuHumOffset < -5.0f) c.htuHumOffset = -5.0f;
                controller->setCalibration(c); break;
            case MenuPage::CALIB_DS_T:
                c.dsTempOffset -= 0.1f; if (c.dsTempOffset < -5.0f) c.dsTempOffset = -5.0f;
                controller->setCalibration(c); break;
            default: break;
        }
        delay(100);
    }
}

void DisplayUI::updateBacklight() {
    // Если прошло более 30 секунд бездействия - гасим свет
    // ИСКЛЮЧЕНИЕ: Manual Ozone (по ТЗ в ручном режиме подсветка может игнорироваться)
    if (backlightOn && (millis() - lastActivityTime > 30000UL)) {
        if (controller->getState() != SystemState::MANUAL_OZONE) {
            lcd.noBacklight();
            backlightOn = false;
        }
    }
}

void DisplayUI::drawPage() {
    CalibrationData c = controller->getCalibration();
    switch (currentPage) {
        case MenuPage::STATUS_IN:    drawStatusIn(); break;
        case MenuPage::STATUS_OUT:   drawStatusOut(); break;
        case MenuPage::STATUS_RELAY: drawRelayState(); break;
        case MenuPage::SET_TEMP:     drawSetTemp(); break;
        case MenuPage::SET_HUM:      drawSetHum(); break;
        case MenuPage::MANUAL_MODES: drawManualModes(); break;
        case MenuPage::CALIB_BME_T:  drawCalibPage("BME TEMP", c.bmeTempOffset, true); break;
        case MenuPage::CALIB_BME_H:  drawCalibPage("BME HUM", c.bmeHumOffset, false); break;
        case MenuPage::CALIB_HTU_T:  drawCalibPage("HTU TEMP", c.htuTempOffset, true); break;
        case MenuPage::CALIB_HTU_H:  drawCalibPage("HTU HUM", c.htuHumOffset, false); break;
        case MenuPage::CALIB_DS_T:   drawCalibPage("DS TEMP", c.dsTempOffset, true); break;
        case MenuPage::STATS:        drawStats(); break;
        case MenuPage::ERROR_LOG:    drawErrorLog(); break;
    }
}

void DisplayUI::drawStatusIn() {
    SensorData in = sensors->getInside();
    lcd.setCursor(0, 0);
    lcd.print(F("IN: "));
    lcd.print(in.temp, 1);
    lcd.print(F("C "));
    lcd.print(in.rh, 0);
    lcd.print(F("%  "));

    lcd.setCursor(0, 1);
    lcd.print(F("AH:"));
    lcd.print(in.ah, 2);
    lcd.print(F(" DP:"));
    lcd.print(in.dewpoint, 1);
}

void DisplayUI::drawStatusOut() {
    SensorData out = sensors->getOutside();
    lcd.setCursor(0, 0);
    lcd.print(F("OUT:"));
    lcd.print(out.temp, 1);
    lcd.print(F("C "));
    lcd.print(out.rh, 0);
    lcd.print(F("% "));

    lcd.setCursor(0, 1);
    lcd.print(F("AH:"));
    lcd.print(out.ah, 2);
    if (out.temp < 0) lcd.print(F(" FROST"));
}

void DisplayUI::drawRelayState() {
    lcd.setCursor(0, 0);
    lcd.print(F("FAN:"));
    lcd.print(controller->getRelayManager()->getFanState() ? F("ON ") : F("OFF"));
    
    lcd.setCursor(9, 0);
    lcd.print(F("O3:"));
    lcd.print(controller->getRelayManager()->getOzoneState() ? F("ON ") : F("OFF"));

    lcd.setCursor(0, 1);
    lcd.print(F("MODE:"));
    lcd.print((int)controller->getState()); 
}

void DisplayUI::drawSetTemp() {
    lcd.setCursor(0, 0);
    lcd.print(F("SET TARGET TEMP"));
    lcd.setCursor(0, 1);
    lcd.print(F("TEMP: "));
    lcd.print(controller->getTargetTemp(), 1);
    lcd.print(F("C"));
}

void DisplayUI::drawSetHum() {
    lcd.setCursor(0, 0);
    lcd.print(F("SET TARGET HUM"));
    lcd.setCursor(0, 1);
    lcd.print(F("HUM:  "));
    lcd.print(controller->getTargetRh(), 0);
    lcd.print(F("%"));
}

void DisplayUI::drawManualModes() {
    lcd.setCursor(0, 0);
    lcd.print(F("MANUAL START:"));
    lcd.setCursor(0, 1);
    lcd.print(F("UP:FAN  DN:OZONE"));
}

void DisplayUI::drawCalibPage(const char* label, float value, bool isTemp) {
    lcd.setCursor(0, 0);
    lcd.print(F("CALIB: "));
    lcd.print(label);
    lcd.setCursor(0, 1);
    lcd.print(F("OFFSET: "));
    if (value >= 0) lcd.print(F("+"));
    lcd.print(value, 1);
    lcd.print(isTemp ? F("C") : F("%"));
}

void DisplayUI::drawStats() {
    SystemStatistics s = controller->getStats();
    lcd.setCursor(0, 0);
    lcd.print(F("UP:")); lcd.print(s.uptimeMinutes / 60); lcd.print(F("h"));
    lcd.print(F(" F:")); lcd.print(s.fanMinutes / 60); lcd.print(F("h"));

    lcd.setCursor(0, 1);
    lcd.print(F("O3:")); lcd.print(s.ozoneMinutes); lcd.print(F("m"));
    lcd.print(F(" HOLD MENU:RST"));
}

void DisplayUI::drawErrorLog() {
    lcd.setCursor(0, 0);
    lcd.print(F("ERROR STATUS:"));
    lcd.setCursor(0, 1);
    ErrorCode err = controller->getError();
    if (err == ErrorCode::NONE) {
        lcd.print(F("SYSTEM OK"));
    } else {
        lcd.print(F("ERR CODE: "));
        lcd.print((int)err);
        lcd.print(F("  UP:RES"));
    }
}