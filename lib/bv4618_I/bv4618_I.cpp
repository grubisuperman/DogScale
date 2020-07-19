/*
  BV4618_I (I2C) and serial charatcer display controller  ByVac
  Copyright (c) 2011 Jim Spence.  All right reserved.
  www.byvac.com - see terms and conditions for using hardware
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Version 1  June 2011

*/  

#include "Arduino.h"
#include "bv4618_I.h"


// ==============================================================
// I2C PRIVATE
// ==============================================================
// ==============================================================
// **************************************************************
// gets a 16 bit value from the i2c bus
// **************************************************************
int BV4618_I::i2_16bit()
{
int rv;
  Wire.requestFrom(_i2adr, 2); // returns 2 bytes
  rv=Wire.read()*256; // high byte
  rv+=Wire.read(); // low byte
  return rv;
}

// **************************************************************
// sends a command to the I2C ASI interface. Most commands have
// just 1 byte following but some have more. All commands begin
// with 0x1b *** The calling must endTransmission
// **************************************************************
void BV4618_I::cmd(char command)
{
  Wire.beginTransmission(_i2adr);
  Wire.write(0x1b);  // command command
  Wire.write(command);  // always at least 1 bye
  // *** calling must end when finished
}

// **************************************************************
// command where ther is just 1 byte folloing the command, this
// is used many times
// **************************************************************
void BV4618_I::cmd1(char command)
{
  cmd(command);
  Wire.endTransmission();
}

// **************************************************************
// command and one byte follows, this is used many times
// **************************************************************
void BV4618_I::cmd2(char command, char b)
{
  cmd(command);
  Wire.write(b);  // byte following command
  Wire.endTransmission();
}

// ==============================================================
// I2C SETUP + MISCL
// ==============================================================
// ==============================================================
// **************************************************************
// initialsise display. The interrupt pin is used for the keypad
// and goes low when a key is in the buffer
// **************************************************************
BV4618_I::BV4618_I(char i2adr, char int_pin)
{
char j;
  _i2adr=i2adr;  
  _int_pin=int_pin;
  pinMode(_int_pin, INPUT);
  Wire.begin(); // join i2c bus
    for(j=0;j<16;j++)
        keyscancodes[j]=0xff;
}

// alrernative no pin specified
BV4618_I::BV4618_I(char i2adr)
{
char j;
  _i2adr=i2adr;  
  _int_pin=0xff; // no pin flag
  pinMode(_int_pin, INPUT);
  Wire.begin(); // join i2c bus
    for(j=0;j<16;j++)
        keyscancodes[j]=0xff;
  
}

// **************************************************************
// sets lines and cols, i.e. type of display
// **************************************************************
void BV4618_I::setdisplay(char rows, char cols)
{
  // # lines
  cmd2(_CMD_SETLINES,rows);
  delay(30);
  // # charatcers
  cmd2(_CMD_SETCOLS,cols);
}


// **************************************************************
// sends char
// **************************************************************
void BV4618_I::putch(char c)
{
  Wire.beginTransmission(_i2adr);
  Wire.write(c);
  Wire.endTransmission();
}  

// **************************************************************
// sends string
// **************************************************************
void BV4618_I::puts(char *s)
{
char *sp=s;  
  Wire.beginTransmission(_i2adr);
  // send *s as bytes of date
  while(*sp)
    Wire.write(*(sp++));
  Wire.endTransmission();
}  

// ==============================================================
// I2C CURSOR MOVE
// ==============================================================
// ==============================================================

// **************************************************************
// cursor up
// **************************************************************
void BV4618_I::crup()
{
  cmd1(_CMD_CURSORUP);
}

// **************************************************************
// cursor down
// **************************************************************
void BV4618_I::crdown()
{
  cmd1(_CMD_CURSORDOWN);
}

// **************************************************************
// cursor right
// **************************************************************
void BV4618_I::crright()
{
  cmd1(_CMD_CURSORRIGHT);
}

// **************************************************************
// cursor left
// **************************************************************
void BV4618_I::crleft()
{
  cmd1(_CMD_CURSORLEFT);
}

// **************************************************************
// moves cursor to a specified position
// **************************************************************
void BV4618_I::rowcol(char line, char col)
{
  cmd(_CMD_CURSORMOVE);
  Wire.write(line);
  Wire.write(col);
  Wire.endTransmission();
}  

// **************************************************************
// sets line position command for different displays, defaults
// should work with most displays
// **************************************************************
void BV4618_I::lineposition(char line, char pos)
{
  cmd(_CMD_SETLINEPOS); // line position command
  Wire.write(line);
  Wire.write(pos);
  Wire.endTransmission();
}

// **************************************************************
// Back light on/off
// **************************************************************
void BV4618_I::backlight(char blon)
{
  cmd(_CMD_BLOFFON);
  if(blon) Wire.write(1);
  else Wire.write(0);
  Wire.endTransmission();
}

// **************************************************************
// cursor home
// **************************************************************
void BV4618_I::crhome()
{
  cmd1(_CMD_CURSORHOME);
}

// **************************************************************
// get device id, this should be 4618
// fixed ver 1
// **************************************************************
int BV4618_I::deviceid()
{
  Wire.beginTransmission(_i2adr);
  Wire.write(_CMD_START); 
  Wire.write(_CMD_DEVICEID);  // device id comand
  Wire.endTransmission();
  return i2_16bit();
}

// **************************************************************
// get device firmware version, this is a string!!
// fixed ver 1
// **************************************************************
void BV4618_I::version(char *ver)
{
char b, tmp[20];  
  Wire.beginTransmission(_i2adr);
  Wire.write(_CMD_START); 
  Wire.write(_CMD_FIRMWARE);  // firmware comand
  Wire.endTransmission();
  // return as a string in the format x.x
  Wire.requestFrom(_i2adr, 2); // returns 2 bytes
  b=Wire.read(); // major byte
  itoa(b,tmp,10); // convert to string
  *ver=0; // just in case
  strcpy(ver,tmp);
  strcat(ver,".");
  b=Wire.read(); // minor byte
  itoa(b,tmp,10);
  strcat(ver,tmp);
}

// **************************************************************
// Sets I2C address
// **************************************************************
void BV4618_I::setaddress(char newaddress)
{
  cmd(_CMD_SETADDRESS);
  Wire.write(0x55);
  Wire.write(0x37);
  Wire.write(newaddress);
  Wire.endTransmission();
}

// **************************************************************
// reset
// **************************************************************
void BV4618_I::reset()
{
  cmd1(_CMD_RESET);
}

// **************************************************************
// reset EEPROM
// **************************************************************
void BV4618_I::resetEEPROM()
{
  cmd1(_CMD_RESETEEPROM);
}

// **************************************************************
// delay mS
// **************************************************************
void BV4618_I::delayms(char del)
{
  cmd2(_CMD_DELAYMS, del);
}

// **************************************************************
// delay S
// **************************************************************
void BV4618_I::delays(char del)
{
  cmd2(_CMD_DELAYS, del);
}

// **************************************************************
// clear screen
// **************************************************************
void BV4618_I::cls()
{
  cmd1(_CMD_CLS);
  delay(100);
}

// **************************************************************
// clear line from cursor to right
// **************************************************************
void BV4618_I::clright()
{
  cmd1(_CMD_CLSRIGHT);
}

// **************************************************************
// clear line from cursor to left
// **************************************************************
void BV4618_I::clleft()
{
  cmd1(_CMD_CLSLEFT);
}

// **************************************************************
// clear all line
// **************************************************************
void BV4618_I::clall()
{
  cmd1(_CMD_CLSLINE);
}

// ==============================================================
// I2C KEPAD
// ==============================================================
// ==============================================================

// **************************************************************
// set up key scan code array for later use. Constructor sets
// these as all 0xff
// **************************************************************
void BV4618_I::setkeycodes(const char *codes)
{
char j;
    for(j=0;j<16;j++)
        keyscancodes[j]=*(codes++);
}

// **************************************************************
// reads interrupt pin
// **************************************************************
char BV4618_I::keyint()
{
    if(_int_pin == 0xff) return 1;
    return digitalRead(_int_pin);
}

// **************************************************************
// returns number of keys in keypad buffer
// **************************************************************
char BV4618_I::keys()
{
  cmd1(_CMD_KEYKEYS);
  Wire.requestFrom(_i2adr, 1);
  return Wire.read();
}

// **************************************************************
// returns key scan code
// **************************************************************
char BV4618_I::keyscan()
{
  cmd1(_CMD_KEYSCAN);
  Wire.requestFrom(_i2adr, 1);
  return Wire.read();
}

// **************************************************************
// returns key as set by key scan code
// **************************************************************
char BV4618_I::key()
{
char scan, j=15; 
  cmd1(_CMD_KEYSCAN);
  Wire.requestFrom(_i2adr, 1);
  scan=Wire.read();
  if(scan == 0xff) return 0xff;
  while(j+1) {
    if(scan == keyscancodes[j]) return j;
    --j;
  }
  return 0xff;
}

// **************************************************************
// clears key buffer
// **************************************************************
void BV4618_I::clskeybuf()
{
  cmd1(_CMD_KEYCLR);
}

// **************************************************************
// sets keypad debounce, default is 50
// **************************************************************
void BV4618_I::keydebounce(char db)
{
  cmd2(_CMD_KEYDEBOUNCE, db);
}

