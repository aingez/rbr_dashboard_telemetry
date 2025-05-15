// udp_receiver.h
#pragma once
#include <winsock2.h>
#include <cstdint>

bool InitWinsock();
SOCKET CreateUdpSocket();
void receiveUdpPackets(SOCKET sock);
char ParseGearFromPacket(const char* buffer);
int extractIntBuffer(const char* buffer, uint16_t offset);
