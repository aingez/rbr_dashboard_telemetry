#pragma once

#include <string>
#include <vector>
#include <windows.h>

// You may already have this declared elsewhere
std::vector<std::string> getAvailableComPorts();

// Prompts user to select a COM port and tries to open it.
// Returns a valid HANDLE or INVALID_HANDLE_VALUE if failed.
HANDLE SelectAndOpenSerialPort();