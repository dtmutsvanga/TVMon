# TVMon
TVMon is a usage monitoring device for keeping track of TV (or other devices) usage. I use the device to give usage access and 
monitor TV usage in my gaming  shops.

## Functions
1. Remote access control via WiFi + local web-page.
2. Usage data storage ( and retrieval -WIP).
3. Time based usage control.
4. Usage notification i.e when usage time is ending, he device notifies the user(s) by beeping two minutes before time ends.
5. EEPROM (FLASH) usage log storage.
6. Stores time in RTC module

## Hardware
The device is based on the ESP8266 microcontroller. It includes two relays, an RTC module for time tracking, an external EEPROM module, a buzzer
an a 5V PSU. 

## Firmware
The firmware was developed in VS Code using the PlatformIO plugin. 

## Shematic and 3D Models
Schematic was made in Diptrace and 3D models for the case were designed in
Solidworks 2018

