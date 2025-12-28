#include "fsm.h"

FSM::FSM() {
    currentState = State::IDLE;
    stateStartTime = 0;
    stateTimeout = 0;
}

void FSM::init() {
    // Инициализация FSM
}

void FSM::update() {
    // Обновление FSM
}

State FSM::getCurrentState() {
    return State::IDLE;
}

void FSM::setState(State newState) {
    // Установка нового состояния
}

void FSM::handleInput() {
    // Обработка входных данных
}

void FSM::processState() {
    // Обработка текущего состояния
}