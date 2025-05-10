#include <Arduino.h>
#include "led_matrix.h"

unsigned long lastUpdateTime = 0;
const unsigned long timeoutDuration = 3000;
bool displayCleared = false;

void setup()
{
  Serial.begin(115200);
  setupMAX7219();
  char start_text[3] = {'A', 'G', 'Z'};
  for (size_t i = 0; i < sizeof(start_text)/sizeof(start_text[0]); i++) {
    displayChar(start_text[i]);
    delay(300);
    clearDisplay();
    delay(50);
  }
  clearDisplay();
}

void loop()
{
  if (Serial.available() > 0) {
    char gear = Serial.read();
    displayChar(gear);
    lastUpdateTime = millis();  // Update the timer
    displayCleared = false;     // Mark display as active
  }

  // Check for inactivity timeout
  if (!displayCleared && (millis() - lastUpdateTime > timeoutDuration)) {
    clearDisplay();
    displayCleared = true;      // Mark as cleared so we don't repeat
  }
}