
/**
 * DS3231 Library
 * 
 * Copyright 2020 Eugene Mwangi <eugene.mwangi@illuminumgreenhouses.com>
 * Based on previous work by:
 *      Basit Ayantunde: <http://lamarrr.github.io>
 * 
 * Manages communication with DS3231 timer.
*/

#include "ds3231.h"

/* ============================================== Public Methods ========================================================= */

/**
 * @brief Initializes the I2C pointer
*/
DS3231::DS3231(TwoWire *i2cAddr)
{
    _i2c = i2cAddr;
}
/**
 * @brief Sets i2c pointer to null
*/
DS3231::~DS3231()
{
    _i2c = nullptr;
}
/**
 * @brief Sets ds3231 time and date
*/
void DS3231::setDS3231Time(int sec, int min, int hr, int dayOfWeek, int day, int month, int year)
{
    _i2c->beginTransmission(DS3231_I2C_ADDRESS);
    _i2c->write(0);                   // set next input to start at the seconds register
    _i2c->write(decToBcd(sec));       // sets seconds
    _i2c->write(decToBcd(min));       // sets minutes
    _i2c->write(decToBcd(hr));        // sets hours
    _i2c->write(decToBcd(dayOfWeek)); // sets day of week(1=Sunday, 7=Saturday)
    _i2c->write(decToBcd(day));       // sets date(1 to 31)
    _i2c->write(decToBcd(month));     // set month
    _i2c->write(decToBcd(year));      // sets year (0 to 99)
    _i2c->endTransmission();
}
/**
 * @brief Reads time from the DS3231 timer
*/
void DS3231::readDS3231Time(int *sec, int *min, int *hr, int *dayOfWeek, int *day, int *month, int *year)
{
    _i2c->beginTransmission(DS3231_I2C_ADDRESS);
    _i2c->write(0); // sets DS3231 register pointer to 00h
    _i2c->endTransmission();
    _i2c->requestFrom(DS3231_I2C_ADDRESS, 7); //request 7 bytes of data from DS3231 starting from register 00h
    *sec = bcdToDec(_i2c->read() & 0x7f);
    *min = bcdToDec(_i2c->read());
    *hr = bcdToDec(_i2c->read() & 0x3f);
    *dayOfWeek = bcdToDec(_i2c->read());
    *day = bcdToDec(_i2c->read());
    *month = bcdToDec(_i2c->read());
    *year = bcdToDec(_i2c->read());
}
/**
 * @brief Displays time and date
*/
void DS3231::displayTime()
{
    int sec, min, hr, dayOfWeek, day, month, year;
    readDS3231Time(&sec, &min, &hr, &dayOfWeek, &day, &month, &year);
    // send it to the serial monitor
    Serial.print(hr);
    Serial.print(":");
    if (min < 10)
        Serial.print("0");
    Serial.print(min);
    Serial.print(":");
    if (sec < 10)
        Serial.print("0");
    Serial.print(sec);
    Serial.print(" ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.print(year);
    Serial.print(" Day of the Week: ");
    switch (dayOfWeek)
    {
    case 1:
        Serial.println("Sunday");
        break;
    case 2:
        Serial.println("Monday");
        break;
    case 3:
        Serial.println("Tueday");
        break;
    case 4:
        Serial.println("Wednesday");
        break;
    case 5:
        Serial.println("Thursday");
        break;
    case 6:
        Serial.println("Friday");
        break;
    case 7:
        Serial.println("Saturday");
        break;
    default:
        Serial.println(dayOfWeek);
        break;
    }
}

/* ======================================================= Private Methods ============================================== */
/**
 * @brief Converts binary coded decimal to normal decimal numbers
*/
int DS3231::bcdToDec(int val)
{
    return ((val / 16 * 10) + (val % 16));
}
/**
 * @brief Converts normal decimal numbersto binary coded decimal
*/
byte DS3231::decToBcd(int val)
{
    return ((val / 10 * 16) + (val % 10));
}