// serial_port.h
#pragma once
#include <windows.h>
#include <string>

HANDLE InitSerialPort(const std::string& portName);
