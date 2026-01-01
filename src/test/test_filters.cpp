#include "../filters.h"
#include <Arduino.h>

void testFilters() {
    Serial.begin(9600);
    
    delay(1000); // Ждем инициализации Serial
    
    Serial.println("Testing MedianFilter with N=3:");
    MedianFilter<float> medianFilter(3);
    
    float inputs[] = {1.0f, 3.0f, 2.0f, 4.0f, 5.0f, 0.5f};
    int numInputs = sizeof(inputs) / sizeof(inputs[0]);
    
    for (int i = 0; i < numInputs; i++) {
        float output = medianFilter.apply(inputs[i]);
        Serial.print("Input: ");
        Serial.print(inputs[i]);
        Serial.print(", Median Output: ");
        Serial.println(output);
    }
    
    Serial.println();
    
    Serial.println("Testing EMAFilter with alpha=0.2:");
    EMAFilter<float> emaFilter(0.2f);
    
    for (int i = 0; i < numInputs; i++) {
        float output = emaFilter.apply(inputs[i]);
        Serial.print("Input: ");
        Serial.print(inputs[i]);
        Serial.print(", EMA Output: ");
        Serial.println(output);
    }
    
    Serial.println();
    
    Serial.println("Testing sequential application (Median then EMA):");
    MedianFilter<float> seqMedianFilter(3);
    EMAFilter<float> seqEmaFilter(0.2f);
    
    for (int i = 0; i < numInputs; i++) {
        float medianOutput = seqMedianFilter.apply(inputs[i]);
        float finalOutput = seqEmaFilter.apply(medianOutput);
        Serial.print("Input: ");
        Serial.print(inputs[i]);
        Serial.print(", Median: ");
        Serial.print(medianOutput);
        Serial.print(", Final: ");
        Serial.println(finalOutput);
    }
    
    Serial.println();
    
    Serial.println("Testing reset functionality:");
    medianFilter.reset();
    emaFilter.reset();
    
    Serial.println("After reset, applying first input again:");
    float firstOutput = medianFilter.apply(10.0f);
    Serial.print("Median after reset: ");
    Serial.println(firstOutput);
    
    float secondOutput = emaFilter.apply(10.0f);
    Serial.print("EMA after reset: ");
    Serial.println(secondOutput);
}