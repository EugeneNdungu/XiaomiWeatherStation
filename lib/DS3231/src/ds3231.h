/**
 * DS3231 Library
 * 
 * Copyright 2020 Eugene Mwangi <eugene.mwangi@illuminumgreenhouses.com>
 * Based on previous work by:
 *      Basit Ayantunde: <http://lamarrr.github.io>
 * 
 * Manages communication with DS3231 timer.
*/

#ifndef DS3231_H_
#define DS3231_H_

#include <Arduino.h>
#include "Wire.h"

#define DS3231_I2C_ADDRESS 0x68 //Address of the ds3231 timer

class DS3231
{
public:
    DS3231(TwoWire *i2cAddr);
    void displayTime();
    void readDS3231Time(int *sec, int *min, int *hr, int *dayOfWeek, int *day, int *month, int *year);
    void setDS3231Time(int sec, int min, int hr, int dayOfWeek, int day, int month, int year);
    ~DS3231();

private:
    byte decToBcd(int val);
    int bcdToDec(int val);
    TwoWire *_i2c = nullptr;
};

#endif // DS3231_H_