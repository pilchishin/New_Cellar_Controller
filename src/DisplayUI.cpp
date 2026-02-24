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
                // Длинное нажатие: например, сброс ошибки
                // controller->resetError(); 
            }
            menuBtnPressed = false;
        }
    }

    // Кнопки UP/DOWN для навигации или изменения параметров
    if (up && !menuBtnPressed) {
        // Логика инкремента параметров
    }
    if (down && !menuBtnPressed) {
        // Логика декремента параметров
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
    switch (currentPage) {
        case MenuPage::STATUS_IN:    drawStatusIn(); break;
        case MenuPage::STATUS_OUT:   drawStatusOut(); break;
        case MenuPage::STATUS_RELAY: drawRelayState(); break;
        // ... остальные страницы
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
    // Тут можно вывести сокращенное название из SystemState
    lcd.print((int)controller->getState()); 
}