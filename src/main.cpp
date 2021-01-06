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
#include <sstream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <LiquidCrystal_I2C.h>

#define SCAN_TIME 10  // seconds
#define SLEEP_TIME 60 //seconds

boolean METRIC = true; //Set true for metric system; false for imperial

BLEScan *pBLEScan;

void IRAM_ATTR resetModule()
{
  ets_printf("reboot\n");
  esp_restart();
}

float current_humidity = -100;
float previous_humidity = -100;
float current_temperature = -100;
float previous_temperature = -100;
String knownBLEAddresses = "58:2d:34:33:9d:3f";

void initBluetooth();
String convertFloatToString(float f);
float CelciusToFahrenheit(float Celsius);

LiquidCrystal_I2C lcd(0x27, 20, 4);
void display();
RTC_DATA_ATTR float lcd_temp = -100;
RTC_DATA_ATTR float lcd_hum = -100;

void Sim800TestFxn();
void Sim800Init();
void printToSerial();
void sendToThingspeak();

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

void setup()
{

  Serial.begin(115200);
  Serial.println("ESP32 XIAOMI DISPLAY");
  Sim800Init();
  Sim800TestFxn();

  lcd.init();
  // turn on LCD backlight
  lcd.backlight();

  initBluetooth();
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("Scanning...");
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
void Sim800TestFxn()
{
  Serial2.println("AT+CMEE=2"); //Set the ME's result error code
  delay(2000);
  printToSerial();
  Serial2.println("AT+CPIN?"); //Checks for pin status. Can be used to check if sim is inserted(properly) or not.
  delay(2000);
  printToSerial();
  Serial2.println("AT+CSQ"); //Returns signal strength indication. Response= +CSQ: <rssi>,<ber>
  delay(2000);
  printToSerial();
  Serial2.println("AT+COPS=?"); //Checks for available networks
  delay(2000);
  printToSerial();
  Serial2.println("AT+CSCS?"); //Checks for terminal equipment's(TE) ch_set. Can be used to check if antennae is ok.
  delay(2000);
  printToSerial();
}
void Sim800Init() //Function to Send Message
{
  Serial2.begin(9600); // Setting the baud rate of GSM Module
  while (!Serial2.available())
  {
    Serial2.println("AT");
    delay(1000);
    Serial.println("Connecting...");
#if LCD_I2C_ENABLED
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");
#endif //LCD_I2C_ENABLED
  }
  Serial.println("Connected!");
#if LCD_I2C_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected");
#endif //LCD_I2C_ENABLED
  //   Serial2.println("AT+CMGDA=\"DEL ALL\"");
  //   delay(2000);
  //   printToSerial();
  //   Serial2.println("AT+CMGF=1"); // Configuring TEXT mode
  //   delay(1000);
  //   printToSerial();
  //   Serial2.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS
  //   delay(1000);
  //   printToSerial();
  //   Serial2.println("AT+CMGL=\"REC UNREAD\""); // Read Unread Messages
  //   delay(2000);
  //   printToSerial();
  //   /*Writing the phone number to EEPROM*/
  //   //    setPhoneNoEEPROM();
  //   /*Send a message to indicate that the system has restarted*/
  //   Serial.println("System Restart");
  //   Serial2.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  //   delay(1000);
  //   Serial2.println("AT+CMGS=\"+25421460975\"\r"); // Replace x with mobile number
  //                                                  //    Serial2.print("AT+CMGS=");
  //                                                  //    Serial2.print("\"+254721460975\"");
  //                                                  //    Serial2.print("\r");
  //   delay(1000);
  //   Serial2.print("System Restart"); // The SMS text you want to send
  //   delay(100);
  //   Serial2.print((char)26); // ASCII code of CTRL+Z. It marks the end of the text message
  //   delay(1000);
  //   printToSerial();
}
void printToSerial()
{
  while (Serial2.available())
  {
    Serial.write(Serial2.read()); //Forward what Software Serial received to Serial Port
  }
}
void sendToThingspeak()
{
  Serial2.println("AT+CIPSHUT");
  delay(1000);

  Serial2.println("AT+CIPSTATUS");
  delay(2000);

  Serial2.println("AT+CIPMUX=0");
  delay(2000);

  printToSerial();

  Serial2.println("AT+CSTT=\"EQUITEL\""); //start task and setting the APN,
  delay(1000);

  printToSerial();

  Serial2.println("AT+CIICR"); //bring up wireless connection
  delay(3000);

  printToSerial();

  Serial2.println("AT+CIFSR"); //get local IP adress
  delay(2000);

  printToSerial();

  Serial2.println("AT+CIPSPRT=0");
  delay(3000);

  printToSerial();

  Serial2.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\""); //start up the connection
  delay(6000);

  printToSerial();

  Serial2.println("AT+CIPSEND"); //begin send data to remote server
  delay(4000);
  printToSerial();

  String str = "GET https://api.thingspeak.com/update?api_key=R4R0V350EBWOFPCE&field1=" + String(lcd_temp) + "&field2=" + String(lcd_hum);
  Serial.println(str);
  Serial2.println(str); //begin send data to remote server

  delay(4000);
  printToSerial();

  Serial2.println((char)26); //sending
  delay(5000);               //waitting for reply, important! the time is base on the condition of internet
  Serial2.println();

  printToSerial();

  Serial2.println("AT+CIPSHUT"); //close the connection
  delay(100);
  printToSerial();
}
