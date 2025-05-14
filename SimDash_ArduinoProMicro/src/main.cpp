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

RpmLedZone rpmZones[] = {
  {0.4, strip.Color(0, 255, 0)},     // Green zone (0%–40%)
  {0.7, strip.Color(255, 165, 0)},   // Orange zone (40%–70%)
  {1.0, strip.Color(255, 0, 0)}      // Red zone (70%–100%)
};

void showRPM(int rpm) {
  static int lastLedCount = -1;

  int ledCount = map(rpm, 0, MAX_RPM, 0, WS2812B_PIN_NUM_LEDS);

  // Only update if the visual state actually changes
  if (ledCount == lastLedCount) {
    return;
  }
  lastLedCount = ledCount;

  for (int i = 0; i < WS2812B_PIN_NUM_LEDS; i++) {
    if (i < ledCount) {
      float ledPos = (float)i / WS2812B_PIN_NUM_LEDS;
      uint32_t color = strip.Color(255, 255, 255); // fallback color

      for (const auto& zone : rpmZones) {
        if (ledPos <= zone.threshold) {
          color = zone.color;
          break;
        }
      }

      strip.setPixelColor(i, color);
    } else {
      strip.setPixelColor(i, 0); // Off
    }
  }

  strip.show();
}


void setup()
{
  Serial.begin(250000);

  strip.begin();
  strip.show();

  setupMAX7219();
  strip.setBrightness(8);  
  
  for (unsigned int i = 0; i <= MAX_RPM; i += (MAX_RPM / 3)) {
    displayChar('X');
    showRPM(i);
    delay(200);
    clearDisplay();
  }  
  showRPM(0);
}

void loop()
{
  // atleast 5B
  if (Serial.available() >= 5) {
    // 1B Gear
    char gear = Serial.read();

    // 4B RPM
    byte rpmBytes[4];
    for (int i = 0; i < 4; i++) {
      rpmBytes[i] = Serial.read();
    }

    // Promote bytes to uint32_t before shifting
    uint32_t engineRPM = 
        (uint32_t)rpmBytes[0] |
        ((uint32_t)rpmBytes[1] << 8) |
        ((uint32_t)rpmBytes[2] << 16) |
        ((uint32_t)rpmBytes[3] << 24);

    displayChar(gear);
    showRPM(engineRPM);

    lastUpdateTime = millis();
    displayCleared = false;
  }

  // timeout
  if (!displayCleared && (millis() - lastUpdateTime > timeoutDuration)) {
    clearDisplay();
    showRPM(0);
    displayCleared = true;
  }
}