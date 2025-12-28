#include "utils.h"

float Utils::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return 0.0f;
}

int Utils::constrainInt(int value, int min, int max) {
    return 0;
}

float Utils::constrainFloat(float value, float min, float max) {
    return 0.0f;
}

bool Utils::isFloatEqual(float a, float b, float epsilon) {
    return false;
}

float Utils::roundToDecimal(float value, int decimals) {
    return 0.0f;
}

unsigned long Utils::calculateCRC(unsigned char* data, int length) {
    return 0;
}

bool Utils::validateCRC(unsigned char* data, int length, unsigned long expectedCRC) {
    return false;
}

void Utils::delayMs(unsigned long ms) {
    // Задержка в миллисекундах
}

unsigned long Utils::getSystemTime() {
    return 0;
}

char* Utils::floatToString(float value, int decimals) {
    return nullptr;
}

char* Utils::intToString(int value) {
    return nullptr;
}

bool Utils::stringToFloat(const char* str, float& result) {
    return false;
}

bool Utils::stringToInt(const char* str, int& result) {
    return false;
}

void Utils::debugPrint(const char* message) {
    // Отладочный вывод
}

void Utils::debugPrintln(const char* message) {
    // Отладочный вывод с новой строки
}

void Utils::swap(float& a, float& b) {
    // Обмен значениями
}

void Utils::swap(int& a, int& b) {
    // Обмен значениями
}

float Utils::calculateMedian(float* array, int size) {
    return 0.0f;
}

float Utils::calculateAverage(float* array, int size) {
    return 0.0f;
}