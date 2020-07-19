#include <Arduino.h>
#include <Wire.h>
#include <bv4618_I.h>
#include "HX711.h"

#define WINDOW_SIZE 5

// Display BV4618-h, I2C: 0x31
const int LCD_I2C_ADDRESS = 0x31;
char dispLineBuffer_LINE1[21] = "";
char dispLineBuffer_LINE2[21] = "    --- TARA ---";
char dispLineBuffer_LINE3[21] = "Balance[           ]";
char dispLineBuffer_LINE4[21] = "";
char weight[6];

// HX711 circuit wiring
const int LOADCELL_A_DOUT_PIN = A2;
const int LOADCELL_A_SCK_PIN = A3;
const float LOADCELL_A_SCALE = 159800; // reading / LOADCELL_A_SCALE -> kg

const int LOADCELL_B_DOUT_PIN = A1;
const int LOADCELL_B_SCK_PIN = A0;
const float LOADCELL_B_SCALE = 154000; // reading / LOADCELL_B_SCALE -> kg

// HW objects
BV4618_I lcd(LCD_I2C_ADDRESS);

HX711 scale_A;
HX711 scale_B;

// data structure
typedef struct
{
  double values[WINDOW_SIZE];
  double values_average;
  double weight_average;
} scale_t;

typedef struct
{
  scale_t A;
  scale_t B;
  double weightSum_AB;
  float balance_AB;
} scaleData_t;

static scaleData_t scaleData;

void updateScaleData(scaleData_t &scaleData)
{
  static byte curIdx = 0;
  //Serial.print("curIdx = ");
  //Serial.println(curIdx);

  scaleData.A.values[curIdx] = scale_A.get_value();
  scaleData.B.values[curIdx] = scale_B.get_value();

  //Serial.print("scalevalue_A = ");
  //Serial.println(scaleData.A.values[curIdx]);

  double sumA = 0;
  double sumB = 0;
  for (byte ctr = 0; ctr < WINDOW_SIZE; ctr++)
  {
    sumA += scaleData.A.values[ctr];
    sumB += scaleData.B.values[ctr];
  }

  scaleData.A.values_average = sumA / WINDOW_SIZE;
  scaleData.A.weight_average = scaleData.A.values_average / LOADCELL_A_SCALE;

  scaleData.B.values_average = sumB / WINDOW_SIZE;
  scaleData.B.weight_average = scaleData.B.values_average / LOADCELL_B_SCALE;

  scaleData.weightSum_AB = scaleData.A.weight_average + scaleData.B.weight_average;
  scaleData.balance_AB = scaleData.A.weight_average / scaleData.weightSum_AB;

  if (abs(scaleData.weightSum_AB) > 0.1)
  {
    if (scaleData.weightSum_AB > 0 && scaleData.balance_AB < 0)
    {
      // load on B
      scaleData.balance_AB = 0;
    }

    if (scaleData.weightSum_AB < 0)
    {
      if (scaleData.balance_AB < 0)
      {
        // A positive, weightSum_AB negative
        // load on A
        scaleData.balance_AB = 1;
      }
      else
      {
        // A, B negative -> invert ratio
        scaleData.balance_AB = 1 - scaleData.balance_AB;
      }
    }
  }
  else
  {
    // weightSum_AB to small
    scaleData.balance_AB = 0.5;
  }

  // set curIdx for next run
  curIdx++;
  if (curIdx == WINDOW_SIZE)
  {
    curIdx = 0; // reset to 0
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing HX711 scales");

  scale_A.begin(LOADCELL_A_DOUT_PIN, LOADCELL_A_SCK_PIN);
  scale_B.begin(LOADCELL_B_DOUT_PIN, LOADCELL_B_SCK_PIN);

  Serial.println("Config LCD");
  // config lcd to remove cursor
  Wire.begin();
  Wire.beginTransmission(LCD_I2C_ADDRESS);
  Wire.write(0x1b);
  Wire.write(0x01);
  Wire.write(0x0c);
  Wire.endTransmission();
  Wire.end();

  Serial.println("Initializing LCD");
  lcd.setdisplay(4, 20);
  lcd.cls();
  lcd.rowcol(2, 1);
  lcd.puts(dispLineBuffer_LINE2);

  while (!scale_A.is_ready())
  {
    Serial.println("Waiting for HX711 scale_A to become ready");
    delay(50);
  }
  Serial.println("TARA HX711 scale_A");
  scale_A.tare();

  while (!scale_A.is_ready())
  {
    Serial.println("Waiting for HX711 scale_B to become ready");
    delay(50);
  }
  Serial.println("TARA HX711 scale_B");
  scale_B.tare();
  strcpy(dispLineBuffer_LINE2, "   --- BEREIT ---   ");
  lcd.rowcol(2, 1);
  lcd.puts(dispLineBuffer_LINE2);
  delay(100);
  strcpy(dispLineBuffer_LINE2, "Gewicht:   00.00 Kg ");
  lcd.rowcol(2, 1);
  lcd.puts(dispLineBuffer_LINE2);

  lcd.rowcol(3, 1);
  lcd.puts(dispLineBuffer_LINE3);
}

void loop()
{

  updateScaleData(scaleData);

  // disp weight
  dtostrf(scaleData.weightSum_AB, 6, 2, weight);
  memcpy(dispLineBuffer_LINE2 + 10, weight, 6);
  lcd.rowcol(2, 1);
  lcd.puts(dispLineBuffer_LINE2);

  // disp balance
  byte indicatorPosition = round((1 - scaleData.balance_AB) * 11 + 0.5);
  if (indicatorPosition < 1)
  {
    indicatorPosition = 1;
  }
  if (indicatorPosition > 11)
  {
    indicatorPosition = 11;
  }
  char balance[11] = "          ";
  balance[indicatorPosition - 1] = 0x2B; //https://en.wikipedia.org/wiki/VT100_encoding
  memcpy(dispLineBuffer_LINE3 + 8, balance, 11);
  lcd.rowcol(3, 1);
  lcd.puts(dispLineBuffer_LINE3);

  // Serial output
  Serial.print("HX711 reading A: ");
  Serial.print(scaleData.A.values_average);
  Serial.print("   reading B: ");
  Serial.print(scaleData.B.values_average);
  Serial.print("   Gewicht A&B: ");
  Serial.print(scaleData.weightSum_AB);
  Serial.print("   Balance A to B: ");
  Serial.print(scaleData.balance_AB);
  Serial.print("  indicatorPosition: ");
  Serial.println(indicatorPosition);

  delay(100);
}