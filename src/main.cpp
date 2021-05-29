/**
 * Xiaomi Weather Station Program
 * 
 * Copyright 2020 Eugene Mwangi <mwangieugene@agriqautomations.com>
 * Based on previous work by:
 *      educ8s.tv: <http://educ8s.tv/esp32-xiaomi-hack/>
 * 
 * Main Program
 * 
 * // IMPORTANT //
 * You need to install this library as well else it won't work
 * https://github.com/fguiet/ESP32_BLE_Arduino
*/

#include <Arduino.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_system.h"

#include "BLEProgram.h"
#include "GSMHeader.h"
#include "SDCard.h"
#include "config.h"

#include <LiquidCrystal_I2C.h>
#if BLE_ENABLE
#define SCAN_TIME 10   // seconds
#define SLEEP_TIME 300 //seconds
#endif
void IRAM_ATTR resetModule()
{
  ets_printf("reboot\n");
  esp_restart();
}

LiquidCrystal_I2C lcd(0x27, 20, 4);
void display();
void writeToSd();

void setup()
{

  Serial.begin(115200);
  Serial.println("ESP32 XIAOMI DISPLAY");
#if GMS_ENABLE
  Sim800Init();
  Sim800TestFxn();
#endif
  lcd.init();
  // turn on LCD backlight
  // lcd.backlight();
#if BLE_ENABLE
  initBluetooth();
#endif
#if SD_ENABLE
  if (!SD.begin(CS_PIN))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  else
  {
    Serial.println("Card Mount Successfull");
  }

  writeFile(SD, "/datalog_ESP32.txt", "Time, Humidity, Temperature \r\n");

  delay(1000);

  readFile(SD, "/datalog_ESP32.txt");
#endif
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("Scanning...");
#if BLE_ENABLE
  char printLog[256];
  // display();
  Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
  // BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
  // pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  // pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  int count = foundDevices.getCount();
  printf("Found device count : %d\n", count);
  Serial.printf("temperature: %.2f humidity: %.2f\n", current_temperature, current_humidity);
#endif
  // pBLEScan->clearResults();

  delay(100);

#if SLEEP_TIME > 0
  if (current_temperature != -100 && current_humidity != -100)
  {
    display();
    sendToThingspeak();
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000); // translate second to micro second
    Serial.printf("Enter deep sleep for %d seconds...\n", (SLEEP_TIME));
    esp_deep_sleep_start();
  }

#endif
}

/**
 * @brief Displays the temperature and humidity on the lcd
*/
void display()
{
  lcd.clear();
  //Print the temperature on the lcd
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.print(lcd_temp);
  lcd.print("C");
  //print the humidity on the lcd
  lcd.setCursor(0, 2);
  lcd.print("Humidity: ");
  lcd.print(lcd_hum);
  lcd.print("%");
}

void writeToSd(){
  
}
