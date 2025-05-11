#include <Arduino.h>
#include "led_matrix.h"
#include <Adafruit_NeoPixel.h>

#define WS2812B_PIN 8       
#define WS2812B_PIN_NUM_LEDS 18 

Adafruit_NeoPixel strip(WS2812B_PIN_NUM_LEDS, WS2812B_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastUpdateTime = 0;
const unsigned long timeoutDuration = 3000;
bool displayCleared = false;

uint16_t MAX_RPM = 10000;
uint16_t REDLINE_RPM = 9500;
int currentRPM = 0;
int step = 100;

// starter
unsigned long previousMillis = 0;
const unsigned long charInterval = 350; // 300 + 50 ms
const unsigned long rpmInterval = 200;

char start_text[3] = {'A', 'G', 'Z'};
size_t charIndex = 0;
bool showingChars = true;

unsigned int rpmValue = 0;
const unsigned int rpmStep = MAX_RPM / 3;
bool showingRPM = false;
// starter

// LUT entry structure
struct RpmLedZone {
  float threshold;    // Percent of redline (0.0–1.0)
  uint32_t color;     // LED color
};

void showRPM(int rpm) {
  int ledCount = map(rpm, 0, MAX_RPM, 0, WS2812B_PIN_NUM_LEDS);

  for (int i = 0; i < WS2812B_PIN_NUM_LEDS; i++) {
    if (i < ledCount) {
      // Color based on zone
      if (i < WS2812B_PIN_NUM_LEDS * 0.4) {
        strip.setPixelColor(i, strip.Color(0, 255, 0));   // Green zone
      } else if (i < WS2812B_PIN_NUM_LEDS * 0.7) {
        strip.setPixelColor(i, strip.Color(255, 165, 0)); // Orange zone
      } else {
        strip.setPixelColor(i, strip.Color(255, 0, 0));   // Red zone
      }
    } else {
      strip.setPixelColor(i, 0); // Off
    }
  }

  strip.show();
}


void setup()
{
  Serial.begin(115200);

  strip.begin();
  strip.show();

  setupMAX7219();
  char start_text[3] = {'A', 'G', 'Z'};
  for (size_t i = 0; i < sizeof(start_text)/sizeof(start_text[0]); i++) {
    displayChar(start_text[i]);
    delay(300);
    clearDisplay();
    delay(50);
  }
  clearDisplay();

  strip.setBrightness(128);  
  for (unsigned int i = 0; i <= MAX_RPM; i += (MAX_RPM / 3)) {
    showRPM(i);
    delay(200);
  }
  showRPM(0);
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