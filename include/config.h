/**
 * Configuration Header File
 * 
 */

#ifndef CONFIG_H
#define CONFIG_H

#define BLE_ENABLED 1 // If enabled(set to > 0), all ble functionalites are enabled
#define GSM_ENABLED 1 // If enabled(set to > 0), all gsm functionalites are enabled
#define SD_ENABLED 1  // If enabled(set to > 0), all sd functionalites are enabled
#define RTC_ENABLED 1 // If enabled(set to > 0), all rtc functionalites are enabled
#define SET_RTC_TIME_ENABLED 0 // If enabled(set to > 0), one can set the rtc's time
#define SD_FILE_INIT_ENABLED 0 // If enabled(set to > 0), new file is created in sd card

//Define the SDA & SCL Pins and the CLOCK Speed of the DS3231 
#define SDA_PIN          21 //D4
#define SCL_PIN          22  //D0
#define CLK_SPEED        400000L // sets I2C clock to 400kHz
#endif // CONFIG_H
