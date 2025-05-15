// globals.cpp
#include "globals.h"

std::mutex dataMutex;
char gearChar = 'N';
int engineRPM = 0;
bool dataReady = false;
