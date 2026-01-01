#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"

// Создаем экземпляр датчика HTU21DF напрямую
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void testHTU21DF() {
  Serial.begin(9600);
  Serial.println("HTU21DF Sensor Test");
  
  // Инициализация датчика
  if (htu.begin()) {
    Serial.println("HTU21DF sensor initialized successfully");
  } else {
    Serial.println("Failed to initialize HTU21DF sensor");
    return; // Вместо остановки, просто выходим
  }

  // Считываем показания температуры и влажности
  float temperature = htu.readTemperature();
  float humidity = htu.readHumidity();
  
  // Проверяем корректность значений
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from HTU21DF sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}