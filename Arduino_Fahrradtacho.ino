//LCD:
//#include <LiquidCrystal.h>
#include "Wire.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>
#define DHTPIN 4
#define DHTTYPE    DHT11
const int chipSelect = 6;
int cnt=0, bew=0, bew2=0, temp, hum, braketemproh, braketemp;
float xAvg, yAvg, zAvg, xMax, yMax, zMax, xMin, yMin, zMin;
LiquidCrystal_I2C lcd(0x27,20,4);
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

void setup() {
  //Serial.begin(9600);
  lcd.init();
  dht.begin();
  pinMode(13, OUTPUT);
  pinMode(5, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  analogReference(EXTERNAL);
  SD.begin(chipSelect);
  File dataFile = SD.open("Tacho.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println("X-Beschleunigung,Y-Beschleunigung,Z-Beschleunigung,Aussentemperatur,Bremsentemperatur");
    dataFile.close();
  }
}

void loop() {
  //Hintergrundbeleuchtung
  if (digitalRead(7) == LOW) {
    lcd.backlight();
    }
  else {
    lcd.noBacklight();
    }
  
  //Bewegungserkenung, anzeige von Max/Min-Werten:
  if (digitalRead(5) == LOW) {
    bew++;
    bew2=0;
    }
  else {
    bew2++;
    }

  if (bew2 >= 500) {bew2 = 300;}
  
  //Beschleunigung:
  long xScaled = map(analogRead(A0), 404, 615, -1000, 1000);
  float xAccel = xScaled / 1000.0;
  xAvg += xAccel;
  
  long yScaled = map(analogRead(A1), 402, 612, -1000, 1000);
  float yAccel = yScaled / 1000.0;
  yAvg += yAccel;
  
  long zScaled = map(analogRead(A2), 411, 616, -1000, 1000);
  float zAccel = zScaled / 1000.0;
  zAvg += zAccel;

  //Temperaturen:
  braketemproh = analogRead(A3);
  braketemp = map(braketemproh, 0, 480, 0, 150);

  //Min/Max-Beschleunigung im RAM speichern:
  if (xAccel > xMax) {xMax = xAccel;}
  if (yAccel > yMax) {yMax = yAccel;}
  if (zAccel > zMax) {zMax = zAccel;}
  
  if (xAccel < xMin) {xMin = xAccel;}
  if (yAccel < yMin) {yMin = yAccel;}
  if (zAccel < zMin) {zMin = zAccel;}


  //Aktualwerte anzeigen:
  if (bew > 0 || bew2 < 300) {
    if (xAccel < 0) {
      lcd.setCursor(0, 0);
      lcd.print(" ");  
      }
    else {
      lcd.setCursor(0, 0);
      lcd.print("  ");
      }
    lcd.print(xAccel,1);
    
    if (yAccel < 0) {
      lcd.setCursor(5, 0);
      lcd.print(" ");
      }
    else {
      lcd.setCursor(5, 0);
      lcd.print("  ");
      }
    lcd.print(yAccel,1);
    
    if (zAccel < 0) {
      lcd.setCursor(10, 0);
      lcd.print(" ");
      }
    else {
      lcd.setCursor(10, 0);
      lcd.print("  ");
      }
    lcd.print(zAccel,1);
    lcd.print(" ");
  }
  else {
    //Min/Max-Werte anzeigen:
    if (bew2 >= 300 && bew2 < 400) {
      lcd.setCursor(0, 0);
      lcd.print("P  ");
      lcd.print(xMax,1);
      lcd.print("  ");
      lcd.print(yMax,1);
      lcd.print("  ");
      lcd.print(zMax,1);
      }
    if (bew2 >= 400 && bew2 < 500) {
      lcd.setCursor(0, 0);
      lcd.print("N ");
      lcd.print(xMin,1);
      lcd.print(" ");
      lcd.print(yMin,1);
      lcd.print(" ");
      lcd.print(zMin,1);
      lcd.print("   ");
      }
    }

  //LCD, zweite Zeile:
  lcd.setCursor(0, 1);
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C ");
  lcd.print(hum);
  lcd.print("% ");
  lcd.print(braketemp);
  lcd.print((char)223);
  lcd.print("C ");
  
  //Bremslicht:
  if (xScaled > 200) {
    digitalWrite(13, HIGH);
    lcd.print(" B");
    }
  else {
    digitalWrite(13, LOW);
    lcd.print("  ");
    }

  cnt++;
  
  if (cnt==100) {
    //lcd.clear();
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    temp = (int)(event.temperature);
    dht.humidity().getEvent(&event);
    hum = (int)(event.relative_humidity);

    //SD:
    String dataString = "";
    dataString += String((xAvg)/200, 2);
   // Serial.print(xAvg/200);
   // Serial.print("\t ");
    dataString += ",";
    dataString += String((yAvg/200), 2);
   // Serial.print(yAvg/200);
   // Serial.print("\t ");
    dataString += ",";
    dataString += String((zAvg/200), 2);
   // Serial.println(zAvg/200);
    dataString += ",";
    dataString += String(temp, DEC);
    dataString += ",";
    dataString += String(braketemp, DEC);
    File dataFile = SD.open("Tacho.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
    
    cnt=0;
    bew=0;
    xAvg=0;
    yAvg=0;
    zAvg=0;
  }
} 
