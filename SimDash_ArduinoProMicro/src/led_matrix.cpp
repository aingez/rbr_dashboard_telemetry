#include <Arduino.h>
#include "led_matrix.h"

#define DATA_PIN 16
#define CLK_PIN 15
#define CS_PIN 10

void sendSPI(byte address, byte value)
{
    digitalWrite(CS_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, address);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, value);
    digitalWrite(CS_PIN, HIGH);
}

void setupMAX7219()
{
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(CS_PIN, OUTPUT);

    sendSPI(0x0F, 0x00); // Display test off
    sendSPI(0x0C, 0x01); // Shutdown mode off
    sendSPI(0x0B, 0x07); // Scan limit = 8 LEDs
    sendSPI(0x0A, 0x01); // Intensity
    sendSPI(0x09, 0x00); // Decode mode = none
    clearDisplay();
}

void clearDisplay()
{
    for (int i = 1; i <= 8; i++)
    {
        sendSPI(i, 0x00);
    }
}

struct Symbol
{
    char c;
    byte glyph[8];
};

static Symbol symbols[] = {
    {'1', {B00011000, B00111000, B00011000, B00011000, B00011000, B00011000, B00011000, B00111100}},
    {'2', {B00111100, B01100110, B00000110, B00001100, B00011000, B00110000, B01100000, B01111110}},
    {'3', {B00111100, B01100110, B00000110, B00011100, B00000110, B00000110, B01100110, B00111100}},
    {'4', {B00001100, B00011100, B00111100, B01101100, B01111110, B00001100, B00001100, B00001100}},
    {'5', {B01111110, B01100000, B01111100, B00000110, B00000110, B00000110, B01100110, B00111100}},
    {'6', {B00111100, B01100000, B01100000, B01111100, B01100110, B01100110, B01100110, B00111100}},
    {'7', {B01111110, B00000110, B00000110, B00001100, B00011000, B00110000, B00110000, B00110000}},
    {'8', {B00111100, B01100110, B01100110, B00111100, B01100110, B01100110, B01100110, B00111100}},
    {'N', {B01100011, B01110011, B01110011, B01111011, B01101111, B01100111, B01100111, B01100011}},
    {'R', {B01111100, B01100110, B01100110, B01111100, B01111000, B01101100, B01100110, B01100110}},
    {'A', {B00011000, B00100100, B01000010, B01000010, B01111110, B01000010, B01000010, B01000010}},
    {'G', {B00111100, B01000010, B10000000, B10000000, B10001110, B10000010, B01000010, B00111100}},
    {'Z', {B01111110, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B01111110}},
    {'X', {B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111}}
};

// create Loop LUT
void displayChar(char c)
{
    for (auto &symbol : symbols)
    {
        if (symbol.c == c)
        {
            for (int i = 0; i < 8; i++)
            {
                sendSPI(i + 1, symbol.glyph[i]);
            }
            return;
        }
    }

    clearDisplay(); // fallback for unknown characters
}