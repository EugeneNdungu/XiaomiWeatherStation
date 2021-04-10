/**
 * BLE Pogram Header File
 * 
 * The header file contains all functions, classes pertaining to the 
 * BLE functionality of the program
 */

#ifndef _BLEPROGRAM_
#define _BLEPROGRAM_

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <sstream>


boolean METRIC = true; //Set true for metric system; false for imperial

BLEScan *pBLEScan;

RTC_DATA_ATTR float lcd_temp = -100;
RTC_DATA_ATTR float lcd_hum = -100;

float current_humidity = -100;
float previous_humidity = -100;
float current_temperature = -100;
float previous_temperature = -100;
String knownBLEAddresses = "58:2d:34:33:9d:3f";

void initBluetooth();
String convertFloatToString(float f);
float CelciusToFahrenheit(float Celsius);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (strcmp(advertisedDevice.getAddress().toString().c_str(), knownBLEAddresses.c_str()) == 0)
    {

      // Serial.printf("test point 2\n");
      int serviceDataCount = advertisedDevice.getServiceDataCount();
      std::string strServiceData = advertisedDevice.getServiceData(0);

      uint8_t cServiceData[100];
      char charServiceData[100];

      strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);

      Serial.printf("\n\nAdvertised Device: %s\n", advertisedDevice.toString().c_str());

      for (int i = 0; i < strServiceData.length(); i++)
      {
        sprintf(&charServiceData[i * 2], "%02x", cServiceData[i]);
      }

      std::stringstream ss;
      ss << "fe95" << charServiceData;

      Serial.print("Payload:");
      Serial.println(ss.str().c_str());

      char eventLog[256];
      unsigned long value, value2;
      char charValue[5] = {
          0,
      };
      switch (cServiceData[11])
      {
      case 0x04:
        sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
        value = strtol(charValue, 0, 16);
        if (METRIC)
        {
          current_temperature = (float)value / 10;
        }
        else
        {
          current_temperature = CelciusToFahrenheit((float)value / 10);
        }
        break;
      case 0x06:
        sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
        value = strtol(charValue, 0, 16);
        current_humidity = (float)value / 10;
        Serial.printf("HUMIDITY_EVENT: %s, %d\n", charValue, value);
        break;
      case 0x0A:
        sprintf(charValue, "%02X", cServiceData[14]);
        value = strtol(charValue, 0, 16);
        Serial.printf("BATTERY_EVENT: %s, %d\n", charValue, value);
        break;
      case 0x0D:
        sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
        value = strtol(charValue, 0, 16);
        if (METRIC)
        {
          current_temperature = (float)value / 10;
        }
        else
        {
          current_temperature = CelciusToFahrenheit((float)value / 10);
        }
        (current_temperature == -100) ?: (lcd_temp = current_temperature);
        Serial.printf("TEMPERATURE_EVENT: %s, %d\n", charValue, value);
        sprintf(charValue, "%02X%02X", cServiceData[17], cServiceData[16]);
        value2 = strtol(charValue, 0, 16);
        current_humidity = (float)value2 / 10;
        Serial.printf("HUMIDITY_EVENT: %s, %d\n", charValue, value2);
        (current_humidity == -100) ?: (lcd_hum = current_humidity);
        break;
      }
    }
  }
};

void initBluetooth()
{
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);
}

String convertFloatToString(float f)
{
  String s = String(f, 1);
  return s;
}

float CelciusToFahrenheit(float Celsius)
{
  float Fahrenheit = 0;
  Fahrenheit = Celsius * 9 / 5 + 32;
  return Fahrenheit;
}

#endif
