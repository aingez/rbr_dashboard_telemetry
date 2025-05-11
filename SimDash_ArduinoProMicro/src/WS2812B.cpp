// WS2812B.cpp
#include "WS2812B.h"

WS2812B::WS2812B(uint16_t num_leds, uint8_t pin)
    : numLEDs(num_leds) {
    pixels = new uint8_t[num_leds * 3];
    memset(pixels, 0, num_leds * 3);
    pinMode(pin, OUTPUT);
    port = portOutputRegister(digitalPinToPort(pin));
    pinMask = digitalPinToBitMask(pin);
}

WS2812B::~WS2812B() {
    delete[] pixels;
}

void WS2812B::begin() {
    noInterrupts();
    digitalWrite(pinMask, LOW);
    interrupts();
}

void WS2812B::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n >= numLEDs) return;
    pixels[n * 3]     = g; // WS2812B expects GRB
    pixels[n * 3 + 1] = r;
    pixels[n * 3 + 2] = b;
}

void WS2812B::clear() {
    memset(pixels, 0, numLEDs * 3);
    show();
}

void WS2812B::show() {
    noInterrupts();
    for (uint16_t i = 0; i < numLEDs; i++) {
        sendPixel(pixels[i * 3 + 1], pixels[i * 3], pixels[i * 3 + 2]);
    }
    interrupts();
    delayMicroseconds(300); // Latch
}

void WS2812B::sendPixel(uint8_t g, uint8_t r, uint8_t b) {
    sendByte(g);
    sendByte(r);
    sendByte(b);
}

void WS2812B::sendByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & (1 << (7 - i))) {
            *port |= pinMask;
            delayMicroseconds(0.8);
            *port &= ~pinMask;
            delayMicroseconds(0.45);
        } else {
            *port |= pinMask;
            delayMicroseconds(0.4);
            *port &= ~pinMask;
            delayMicroseconds(0.85);
        }
    }
}
