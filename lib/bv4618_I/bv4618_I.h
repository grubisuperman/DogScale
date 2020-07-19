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

#ifndef _H_bv4618_h
#define _H_bv4618_h

#define _CMD_START 0x1b
#define _CMD_CURSORUP 0x20
#define _CMD_CURSORDOWN 0x21
#define _CMD_CURSORRIGHT 0x22
#define _CMD_CURSORLEFT 0x23
#define _CMD_CURSORMOVE 0x24
#define _CMD_CURSORHOME 0x25
#define _CMD_SETLINES 0x30
#define _CMD_SETCOLS 0x31
#define _CMD_SETLINEPOS 0x32
#define _CMD_BLOFFON 0x3
#define _CMD_DEVICEID 0x40
#define _CMD_FIRMWARE 0x41
#define _CMD_SETADDRESS 0x42
#define _CMD_RESET 0x43
#define _CMD_RESETEEPROM 0x44
#define _CMD_DELAYMS 0x47
#define _CMD_DELAYS 0x48
#define _CMD_CLS 0x50
#define _CMD_CLSRIGHT 0x51
#define _CMD_CLSLEFT 0x52
#define _CMD_CLSLINE 0x53
// keypad
#define _CMD_KEYKEYS 0x10
#define _CMD_KEYSCAN 0x11
#define _CMD_KEYCLR 0x14
#define _CMD_KEYDEBOUNCE 0x15


#include "Wire.h"

class BV4618_I : public TwoWire
{
  public:
    BV4618_I(char i2adr, char int_pin);
    BV4618_I(char i2adr);
    void setdisplay(char cols, char rows);
    void putch(char c);
    void puts(char *s);
    void crup();
    void crdown();
    void crright();
    void crleft();
    void rowcol(char line, char col);
    void lineposition(char line, char pos);
    void backlight(char blon);
    void crhome();
    int deviceid();
    void version(char *ver);
    void setaddress(char newaddress);
    void reset();
    void resetEEPROM();
    void delayms(char del);
    void delays(char del);
    void cls();
    void clright();
    void clleft();
    void clall();
    void setkeycodes(const char *codes);
    char keyint();
    char keys();
    char keyscan();
    char key();
    void clskeybuf();
    void keydebounce(char db);
  private:
    char _i2adr, _int_pin;
    char keyscancodes[16];
    int i2_16bit();
    void cmd(char command);
    void cmd1(char command);
    void cmd2(char command, char b);
};
  
#endif
