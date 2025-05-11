// WS2812B.h
#ifndef WS2812B_H
#define WS2812B_H

#include <Arduino.h>

class WS2812B {
public:
    WS2812B(uint16_t num_leds, uint8_t pin);
    ~WS2812B();
    
    void begin();
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void show();
    void clear();

private:
    uint8_t* pixels;
    uint16_t numLEDs;
    uint8_t pinMask;
    volatile uint8_t* port;

    void sendByte(uint8_t byte);
    void sendPixel(uint8_t r, uint8_t g, uint8_t b);
};

#endif
