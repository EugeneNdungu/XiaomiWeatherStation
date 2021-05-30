/**
 * Xiaomi Weather Station Program
 * 
 * Copyright 2020 Eugene Mwangi <mwangieugene@agriqautomations.com>
 * Based on previous work by:
 *      educ8s.tv: <http://educ8s.tv/esp32-xiaomi-hack/>
 *      ElectronicWings: <https://www.electronicwings.com/users/SUJANAABIRAMIC/projects/1086/smart-agriculture-using-iot-technology>
 * 
 * Main Program
 * 
 * // IMPORTANT //
 * You need to install this library as well else it won't work
 * https://github.com/fguiet/ESP32_BLE_Arduino
 * 
 * 
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
#include "ds3231.h"

#if BLE_ENABLED
#define SCAN_TIME 10   // seconds
#define SLEEP_TIME 300 //seconds
#endif                 // BLE_ENABLED

#if RTC_ENABLED
DS3231 *rtc = new DS3231(&Wire);
#endif // RTC_ENABLED

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
#if GSM_ENABLED
  Sim800Init();
  Sim800Debug();
#endif // GSM_ENABLE
  lcd.init();
  // turn on LCD backlight
  // lcd.backlight();
#if BLE_ENABLED
  initBluetooth();
#endif // BLE_ENABLE
#if SD_ENABLED
  if (!SD.begin(CS_PIN))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  else
  {
    Serial.println("Card Mount Successfull");
  }
#if SD_FILE_INIT_ENABLED
  writeFile(SD, "/datalog_ESP32.txt", "Date, Time, Humidity, Temperature \r\n");
#endif // SD_FILE_INIT_ENBLED
  delay(1000);

  // readFile(SD, "/datalog_ESP32.txt");
#endif // SD_ENABLE
#if RTC_ENABLED
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(CLK_SPEED);
#if SET_RTC_TIME_ENABLED
  //sec, min, hr, dayOfWeek(1=Sunday, 7=Saturday), dayOfMonth(1-31), month, year(0-99)
  rtc->setDS3231Time(47, 45, 23, 7, 29, 5, 21);
#endif // SET_RTC_TIME_ENABLED
#endif // RTC_ENABLED
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("Scanning...");
#if BLE_ENABLED
  // char printLog[256];
  // display();
  Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
  // BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
  // pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  // pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  int count = foundDevices.getCount();
  printf("Found device count : %d\n", count);
  Serial.printf("temperature: %.2f humidity: %.2f\n", current_temperature, current_humidity);
#endif // BLE_ENABLED
  // pBLEScan->clearResults();

  delay(100);

#if SLEEP_TIME > 0
  if (current_temperature != -100 && current_humidity != -100)
  {
    display();
    writeToSd();
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

/**
 * @brief Stores data to the SD Card 
 */
void writeToSd()
{
  String dataString = "";
#if RTC_ENABLED
  // Get data from the DS3231
  int second, minute, hour, dayOfWeek, day, month, year;
  // retrieve data from DS3231
  rtc->readDS3231Time(&second, &minute, &hour, &dayOfWeek, &day, &month, &year);
  rtc->displayTime();
#endif //RTC_ENABLED
  //Concatenate the date to the String(Day/Month/Year)
  dataString += String(day);
  dataString += "/";
  dataString += String(month);
  dataString += "/";
  dataString += String(year);
  //Separate with comma
  dataString += ",";
  //Concatenate the time(hr:min:sec)
  dataString += String(hour);
  dataString += ":";
  if (minute < 10)
    dataString += "0";
  dataString += String(minute);
  dataString += ":";
  if (second < 10)
    dataString += "0";
  dataString += String(second);
  //Separate with comma
  dataString += ",";
  //Concatenate the temperature and humidity
  dataString += String(current_humidity);
  dataString+="%";
  dataString += ",";
  dataString += String(current_temperature);
  dataString += "C\r\n";

  Serial.println(dataString);
  appendFile(SD, "/datalog_ESP32.txt", dataString.c_str());
}
