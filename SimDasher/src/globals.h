// globals.h
#pragma once
#include <mutex>

extern std::mutex dataMutex;
extern char gearChar;
extern int engineRPM;
extern bool dataReady;
