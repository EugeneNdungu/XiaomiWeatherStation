/**
 * SD Card Header File
 * 
 * The header file contains all functions definitions pertaining to the 
 * sd card functionality of the program
 * Based on previous work by:
 *      Christopher: <https://diyi0t.com/sd-card-arduino-esp8266-esp32/>
 */

#ifndef _SDCARD_
#define _SDCARD_

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define CS_PIN 5

/**
 * @brief Function opens a file in the SD Card with write permission
 * @param fs: pointer to the memory address of the file system from the SD card (global namespace)
 * @param path: pointer to the path of the text file
 * @param message: string to be written to the SD Card
 */
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

/**
 * @brief Function opens a file in the SD Card with write permission
 * @param fs: pointer to the memory address of the file system from the SD card (global namespace)
 * @param path: pointer to the path of the text file
 * @param message: string to be written to the SD Card
 */
void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("Message appended");
    }
    else
    {
        Serial.println("Append failed");
    }
    file.close();
}

/**
 * @brief Function opens a file in the SD Card with read permission
 * @param fs: pointer to the memory address of the file system from the SD card (global namespace)
 * @param path: pointer to the path of the text file
 */
void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

#endif // _SDCARD_