#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"
#include "sensors.h"

// Простой тест для проверки корректности изменений
void setup() {
  Serial.begin(9600);
  
  // Создание экземпляра HTU21DSensor
  HTU21DSensor sensor;
  
  // Инициализация датчика
  if (sensor.begin()) {
    Serial.println("HTU21D sensor initialized successfully");
    
    // Получение значений
    float temperature = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
  } else {
    Serial.println("Failed to initialize HTU21D sensor");
  }
}

void loop() {
  // Пустой цикл для теста
}