/*
 *
 *
 *    MeterSWR_LCD.ino
 *
 *    MeterSWR modified for output to lcd2004 i2c Display
 *    by KB1WAH as a tutorial for Waltham Amature Radio Club
 *    Based on work of Matthew K. Roberts, KK5JY
 *
 *    Orignal Serial Based code modified from:
 *
 *    License: GNU General Public License Version 3.0.
 *
 *    Copyright (C) 2014 by Matthew K. Roberts, KK5JY. All rights reserved.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see: http://www.gnu.org/licenses/
 *
 *
 */

// ================== START CONFIGURATION SETTINGS ==================
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
// Enable bar graph; LED pin definitions below
//#define SWR_BAR_GRAPH

// Maximum Arduino A/D reading; used only to scale the power readings.
#define ARDUINO_AD_MAX (264)

// define power limits (W); this is the power reading corresponding
//    to a full-scale A/D reading; to report the power as a percentage
//    of full scale, set this to 100.0.
#define FULL_SCALE_FORWARD (100.0)
#define FULL_SCALE_REFLECTED (100.0)

// Define integration windows (set either/both to zero to disable); if either
//   of these is non-zero, the corresponding reading will be averaged over
//   the specified number of readings.  This helps to smooth the response of
//   the meter when the inputs are noisy or jittery, or for modes that have
//   some very short transients that may produce unnecessarily high SWR readings.
#define SWR_WINDOW (4)
#define PWR_WINDOW (4)

// MINIMUM Power Level - this is the minimum A/D reading that will produce an
//   SWR != 1.0; this helps to prevent unnecessarily high SWR readings at low'
//   power levels where there is insufficient A/D resolution to make accurate
//   SWR calculations.  This value is the RAW value used as a minimum FORWARD
//   power limit.  Below this RAW value, the SWR will be reported as 1.0.
#define MIN_POWER (5)

//
//  Serial Port - assign a specific hardware port to be the interface to
//                the host; by default, the USB connection on most boards
//                is 'Serial', which is also the hardware TTL serial port
//                on the UNO board.  Make sure to only select ONE option.
//

// default for most boards' USB port
#define HostSerial Serial

// default for Leonardo UART, and second MEGA port
//#define HostSerial Serial1

// default for third MEGA port
//#define HostSerial Serial2

// default for fourth MEGA port
//#define HostSerial Serial3

// the 'baud rate' for the serial port
#define SerialRate (9600)

//
//  configuration for the 'soft' bar graph (if enabled)
//
#ifdef SWR_BAR_GRAPH
#include "LightBar.h"
const int BarLength = 11;
const LightBarItem barItems[BarLength] = {
  { 1.01,  2, false },
  { 1.10,  3, false },
  { 1.30,  4, false },
  { 1.50,  5, false },
  { 1.75,  6, false },

  { 2.00,  7, false },
  { 2.33,  8, false },
  { 2.66,  9, false },

  { 3.0,  10, false },
  { 4.0,  11, false },
  { 5.0,  12, false },
};
#endif

// ================== END OF CONFIGURATION SETTINGS ==================

// The version string
#define VERSION_STRING ("MeterSWR 1.0 (beta-20140413a)")

// local headers
#include "Elapsed.h"
#include "Bounce2.h"
#include "SWR.h"
#include "pins.h"


//
//  Constants
//

// maximum auto-poll interval (msec)
const int MaxAutoPoll = 1000;

// data LED hang-time (makes it more obvious to the user that data was rec'd)
const int DataLedDelay = 250;


//
//  Global Vars
//

#ifdef SWR_BAR_GRAPH
// the light bar
LightBar Bar(barItems, BarLength);
#endif

// the SWR calculator
SWR swr(FWD_PIN, REF_PIN);

// a string to hold incoming data
String inputString = "#VERSION;"; // start with version query

// and another for command data
String cmdText = "";

// command parsing
String command = "";
String argument = "";

// whether the string is complete
boolean inputReady = true;	// read the command

// maximum string lengths
const int InputLength = 16;
const int CommandLength = 16;
const int BufferLength = 8;

// I/O buffer
char ioBuffer[BufferLength];

#ifdef CMD_LED
// the last RX data from the host
unsigned long lastRxData;
bool lastCmdLED = false;
#endif

// echo USB RX data back to host
bool echo = false;

// currently parsing a command
bool cmd = false;

// autopolling
unsigned autoPoll = 0;
unsigned long lastPoll = 0;
bool autoRaw = false;


//
//  SplitCommand(...) - split a command from its argument
//
static void SplitCommand(const String &input, String &cmd, String &arg) {
  bool isCmd = true;
  cmd = "";
  arg = "";
  for (int i = 0; i != input.length(); ++i) {
    char ch = input.charAt(i);
    if (ch == '=') {
      isCmd = false;
      continue;
    }
    if (isCmd) {
      cmd += ch;
    } else {
      arg += ch;
    }
  }
}


//
//  FormatFloat(...) - format a floating point number
//
static void FormatFloat(char *buffer, unsigned len, float value) {
  byte point = 0;
  if (value >= 10) point = 1;
  if (value >= 100) point = 2;

  snprintf(buffer, len, "%04lu", (unsigned long)(value * 1000));
  byte off = strnlen(buffer, sizeof(buffer));
  for (byte i = off; i != point; --i) {
    buffer[i + 1] = buffer[i];
  }
  buffer[point + 1] = '.';
}

LiquidCrystal_I2C	lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
  
//
//  setup()
//
void setup() {
  lcd.begin (20, 4);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); // go home
  lcd.setCursor(0, 0);
  lcd.print(" SIMPLE SWR METER");
  lcd.setCursor(0, 1);
  lcd.print("       by       ");
  lcd.setCursor(0, 2);
  lcd.print("     KB1WAH      ");
  lcd.setCursor(0, 3);
  lcd.print(" Brandon W. Hockle  ");
  // start the serial port
  HostSerial.begin(SerialRate);

  // allocate string storage
  inputString.reserve(InputLength);
  cmdText.reserve(CommandLength);

  // the 'command' LED
#ifdef CMD_LED
  pinMode(CMD_LED, OUTPUT);
  digitalWrite(CMD_LED, LOW);
#endif

  // the A/D inputs
  pinMode(FWD_PIN, INPUT);
  pinMode(REF_PIN, INPUT);

  // configure the SWR algorithm
  swr.MinPower(MIN_POWER);
  swr.WindowSWR(SWR_WINDOW);
  swr.WindowPower(PWR_WINDOW);

#ifdef SWR_BAR_GRAPH
  // initialize the bar graph
  Bar.Initialize();
#endif
}


//
//  loop()
//
void loop() {
  unsigned long now;

  // spend all our spare time reading the transducer
  swr.Poll();
#ifdef SWR_BAR_GRAPH
  Bar.Update(swr.Value());
#endif

#ifdef CMD_LED
  // extinguish the data LED a few ms after the last byte received
  now = millis();
  if (lastCmdLED == HIGH && Elapsed(now, lastRxData) > DataLedDelay) {
    lastCmdLED = LOW;
    digitalWrite(CMD_LED, lastCmdLED);
  }
#endif

  // service auto-polling
  now = millis();
  if (autoPoll && (Elapsed(now, lastPoll) >= autoPoll)) {
    lastPoll = now;

    if (autoRaw) {
      HostSerial.write("#RAW=");
      snprintf(ioBuffer, sizeof(ioBuffer), "%d", swr.ForwardRaw());
      HostSerial.write(ioBuffer);
      HostSerial.write(",");
      snprintf(ioBuffer, sizeof(ioBuffer), "%d", swr.ReflectedRaw());
      HostSerial.write(ioBuffer);
      HostSerial.write(";");
      if (echo) {
        HostSerial.write("\r\n");
      }
    } else {
      FormatFloat(ioBuffer, sizeof(ioBuffer), swr.Value());

      HostSerial.write("#SWR=");
      HostSerial.write(ioBuffer);
      HostSerial.write(';');
      if (echo) {
        HostSerial.write("\r\n");
      }
    }
  }

  // if user input ready
  if (inputReady) {
    // light the CMD LED
#ifdef CMD_LED
    lastRxData = millis();
    if (lastCmdLED == LOW) {
      lastCmdLED = HIGH;
      digitalWrite(CMD_LED, lastCmdLED);
    }
#endif

    // for each char received from the USB...
    for (int i = 0; i != inputString.length(); ++i) {
      char ch = inputString.charAt(i);

      // if we are in command mode...
      if (cmd) {
        if (ch == '#') {
          // if a '#' sent mid-command, start over
          cmdText = "";
        } else if (ch == ';') {
          // terminate and process the command string
          cmd = false;
          bool ok = false;
          if (echo) HostSerial.print(';');

          // parse the command and argument
          SplitCommand(cmdText, command, argument);
          if (command.equalsIgnoreCase("SWR")) {
            //
            //  SWR: read A/D and compute SWR
            //

            // add the SWR value to the response
            cmdText += "=";
            FormatFloat(ioBuffer, sizeof(ioBuffer), swr.Value());
            cmdText += ioBuffer;
            ok = true;
          } else if (command.equalsIgnoreCase("RAW")) {
            //
            //  RAW: read A/D and output raw values
            //
            cmdText += "=";
            snprintf(ioBuffer, sizeof(ioBuffer), "%d", swr.ForwardRaw());
            cmdText += ioBuffer;
            cmdText += ",";
            snprintf(ioBuffer, sizeof(ioBuffer), "%d", swr.ReflectedRaw());
            cmdText += ioBuffer;
            ok = true;
          } else if (command.equalsIgnoreCase("PWR")) {
            //
            //  POWER: read A/D and output scaled values
            //
            cmdText += "=";
            FormatFloat(ioBuffer, sizeof(ioBuffer), (swr.ForwardRaw() * (float)FULL_SCALE_FORWARD) / (float)ARDUINO_AD_MAX);
            cmdText += ioBuffer;
            cmdText += ",";
            FormatFloat(ioBuffer, sizeof(ioBuffer), (swr.ReflectedRaw() * (float)FULL_SCALE_REFLECTED) / (float)ARDUINO_AD_MAX);
            cmdText += ioBuffer;
            ok = true;
          } else if (command.equalsIgnoreCase("ALL")) {
            //
            //  ALL: read A/D and output scaled values + SWR
            //
            cmdText += "=";
            FormatFloat(ioBuffer, sizeof(ioBuffer), swr.Value());
            cmdText += ioBuffer;
            cmdText += ",";
            FormatFloat(ioBuffer, sizeof(ioBuffer), (swr.ForwardRaw() * (float)FULL_SCALE_FORWARD) / (float)ARDUINO_AD_MAX);
            cmdText += ioBuffer;
            cmdText += ",";
            FormatFloat(ioBuffer, sizeof(ioBuffer), (swr.ReflectedRaw() * (float)FULL_SCALE_REFLECTED) / (float)ARDUINO_AD_MAX);
            cmdText += ioBuffer;
            ok = true;
          } else if (command.equalsIgnoreCase("ECHO")) {
            //
            //  QUERY/SET ECHO
            //
            if (argument.length() == 0) {
              ok = true;
              cmdText += "=";
              cmdText += echo ? "1" : "0";
            } else {
              int newEcho = argument.toInt();
              echo = newEcho ? true : false;
              ok = true;
            }
          } else if (command.equalsIgnoreCase("AUTO")) {
            //
            //  QUERY/SET AUTO-POLL
            //
            if (argument.length() == 0) {
              ok = true;
              cmdText += "=";
              cmdText += autoPoll;
            } else {
              int newAuto = argument.toInt();
              if (newAuto >= 0 && newAuto < MaxAutoPoll) {
                autoPoll = newAuto;
                ok = true;
              }
            }
          } else if (command.equalsIgnoreCase("AUTORAW")) {
            //
            //  QUERY/SET AUTO-POLL RAW MODE
            //
            if (argument.length() == 0) {
              ok = true;
              cmdText += "=";
              cmdText += autoRaw ? "1" : "0";
            } else {
              int newAuto = argument.toInt();
              autoRaw = newAuto ? true : false;
              ok = true;
            }
          } else if (command.equalsIgnoreCase("VERSION")) {
            //
            //  QUERY VERSION NUMBER
            //
            ok = true;
            cmdText += "=";
            cmdText += VERSION_STRING;
          }

          //
          //  TODO: process other command contents
          //

          // respond with #OK or #ERR
          if (echo)
            HostSerial.print("\r\n");
          HostSerial.print(ok ? "#OK:" : "#ERR:");
          HostSerial.print(cmdText);
          HostSerial.print(';');
          if (echo)
            HostSerial.print("\r\n");
          cmdText = "";
        } else {
          if (cmdText.length() < CommandLength) {
            cmdText += ch;
          }
          if (echo) {
            HostSerial.print(ch);
          }
        }
        continue;
      }

      // start command mode??
      if ((!cmd) && (ch == '#')) {
        cmd = true;
        if (echo) HostSerial.print('#');
        continue;
      }
    }

done:
    // clear the string:
    inputString = "";
    inputReady = false;
  }

  while (HostSerial.available() && (inputString.length() < 80)) {
    // get the new byte:
    char inChar = (char)HostSerial.read();
    // add it to the inputString:
    inputString += inChar;
    inputReady = true;
  }
        lcd.clear();
        lcd.home();
        lcd.setCursor(0,0);
        lcd.print("fwd_voltage=");
        lcd.print(swr.ForwardRaw());
        
        
        lcd.setCursor(0,1);
        lcd.print("rfl_voltage=");
        lcd.print(swr.ReflectedRaw());
        
        
       
        lcd.setCursor(0,3);
        lcd.print("SWR=");
        lcd.print(swr.Value());
        delay(1000);
}

// EOF
