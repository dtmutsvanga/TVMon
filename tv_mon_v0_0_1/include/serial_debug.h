#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H
#define ENABLE_DEBG

#include "SoftwareSerial.h"
#ifdef ENABLE_DEBG
#define TVMON_DEBUG_INIT(x) Serial.begin(x)
#define TVMON_DEBUG(x) Serial.println(x)
#define TVMON_DEBUG_nln(x)  Serial.print(x)
#else
#define TVMON_DEBUG_INIT(x) void(x)
#define  TVMON_DEBUG(x) (void)x
#define  TVMON_DEBUG_nln void(x)
#endif //ENABLE_DEBG

 #endif // SERIAL_DEBUG
