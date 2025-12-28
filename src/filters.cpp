#include "filters.h"

Filter::Filter() {
    lastOutput = 0.0f;
}

Filter::~Filter() {
    // Виртуальный деструктор
}

void Filter::reset() {
    lastOutput = 0.0f;
}

MovingAverageFilter::MovingAverageFilter(int windowSize) : Filter() {
    this->windowSize = windowSize;
    buffer = new float[windowSize];
    index = 0;
    count = 0;
    sum = 0.0f;
}

float MovingAverageFilter::apply(float input) {
    return 0.0f;
}

void MovingAverageFilter::reset() {
    index = 0;
    count = 0;
    sum = 0.0f;
    for(int i = 0; i < windowSize; i++) {
        buffer[i] = 0.0f;
    }
}

MedianFilter::MedianFilter(int windowSize) : Filter() {
    this->windowSize = windowSize;
    buffer = new float[windowSize];
    index = 0;
    count = 0;
}

float MedianFilter::apply(float input) {
    return 0.0f;
}

void MedianFilter::reset() {
    index = 0;
    count = 0;
    for(int i = 0; i < windowSize; i++) {
        buffer[i] = 0.0f;
    }
}

KalmanFilter::KalmanFilter(float processNoise, float measurementNoise) : Filter() {
    this->processNoise = processNoise;
    this->measurementNoise = measurementNoise;
    estimate = 0.0f;
    error = 0.0f;
    kalmanGain = 0.0f;
}

float KalmanFilter::apply(float input) {
    return 0.0f;
}

void KalmanFilter::reset() {
    estimate = 0.0f;
    error = 0.0f;
    kalmanGain = 0.0f;
}