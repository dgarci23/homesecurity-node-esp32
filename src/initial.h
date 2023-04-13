#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <FS.h>

#ifndef INITIAL_H
#define INITIAL_H


void initSPIFFS();
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
bool initMac();
void initialConfig();
void readFile();
void getBroadcastAddress(uint8_t broadcastAddress[]);

#endif