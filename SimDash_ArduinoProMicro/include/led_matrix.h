#ifndef LED_MATRIX_H
#define LED_MATRIX_H

void setupMAX7219();
void clearDisplay();
void displayChar(char c);
void sendSPI(byte address, byte value);

#endif