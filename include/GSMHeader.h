/**
 * GSM Header File
 * 
 * The header file contains all functions definitions pertaining to the 
 * GSM functionality of the program
 * Based on previous work by:
 *      Alex Newton: <https://how2electronics.com/send-gsm-sim800-900-gprs-data-thingspeak-arduino/>
 */

#ifndef _GSMHEADER_
#define _GSMHEADER_

#include "Arduino.h"

#define SAFARICOM_SIMCARD 0
#define AIRTEL_SIMCARD 1

#if AIRTEL_SIMCARD
    #define APN_VALUE "\"EQUITEL\""
#elif SAFARICOM_SIMCARD
    #define APN_VALUE "\"safaricom\"" 
#endif

extern RTC_DATA_ATTR float lcd_temp;
extern RTC_DATA_ATTR float lcd_hum;


void printToSerial()
{
    while (Serial2.available())
    {
        Serial.write(Serial2.read()); //Forward what Software Serial received to Serial Port
    }
}

void Sim800Debug()
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

void sendToThingspeak()
{
    Serial2.println("AT+CIPSHUT"); // shuts down any IP  connection
    delay(1000);

    Serial2.println("AT+CIPSTATUS"); // queries current connection status
    delay(2000);

    Serial2.println("AT+CIPMUX=0"); // sets a single IP connection
    delay(2000);

    printToSerial();

    //start task and setting the APN,
    Serial2.print("AT+CSTT="); 
    Serial2.println(APN_VALUE);
    delay(1000);

    printToSerial();

    Serial2.println("AT+CIICR"); //bring up wireless connection
    delay(3000);

    printToSerial();

    Serial2.println("AT+CIFSR"); //get local IP adress
    delay(2000);

    printToSerial();

    Serial2.println("AT+CIPSPRT=0"); // sets prompt to "send ok" when module sends data but does not echo when send is successful
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
#endif // _GSMHEADER_