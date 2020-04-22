#include <Servo.h>
#include <Wire.h>
#include "DS1307.h"
#include "Ephemeride.h"

#define CURRENTYEAR 20
const int closeGatePin = 4;
const int openGatePin = 5;
const byte confirmedAttempt = 10;

//Debug purpose
byte testingStruct[4] = {0, 0, 0, 0};

int closeGateState = -1;
int openGateState = -1;
byte moveDuration = 0;

Servo servoGate;
DS1307 tRtc;

void setup()
{
  Serial.begin(9600);
  pinMode(closeGatePin, INPUT_PULLUP);
  pinMode(openGatePin, INPUT_PULLUP);
  servoGate.attach(9);
  
  /* Vérifie si le module RTC est initialisé */
  tRtc = DS1307();
  DateTime_t now;
  if (tRtc.read_current_datetime(&now))
  {
    Serial.println(F("L'horloge du module RTC n'est pas active !"));

    // Reconfiguration avec une date et heure en dure (pour l'exemple)
    now.seconds = 0;
    now.minutes = 05;
    now.hours = 11; // 12h 0min 0sec
    now.is_pm = 0;
    now.day_of_week = 3;
    now.days = 21;
    now.months = 04;
    now.year = 20; // 1 dec 2016
    tRtc.adjust_current_datetime(&now);
  }
  else
  {
    Serial.println(F("L'horloge du module RTC est active !"));
    DateTime_t now;
    tRtc.read_current_datetime(&now);
    /* Affiche la date et heure courante */
    Serial.print(F("Date : "));
    Serial.print(now.days);
    Serial.print(F("/"));
    Serial.print(now.months);
    Serial.print(F("/"));
    Serial.print(now.year + 2000);
    Serial.print(F("  Heure : "));
    Serial.print(now.hours);
    Serial.print(F(":"));
    Serial.print(now.minutes);
    Serial.print(F(":"));
    Serial.println(now.seconds);
  }
  Serial.println("Configuration finished !");

  if (closeGateState == -1 && openGateState == -1)
  {
    Serial.println("Initial value for closegate: ");
    Serial.println(closeGateState);
    Serial.println("Initial value for moveGate: ");
    Serial.println(openGateState);
    closeGateState = digitalRead(closeGatePin);
    openGateState = digitalRead(openGatePin);
    Serial.println("After init value for closegate: ");
    Serial.println(closeGateState);
    Serial.println("After init value for openGate: ");
    Serial.println(openGateState);

    // for debuging purpose only, set a ramdom position of the gate
    // servoGate.attach(9);
    // servoGate.write(115);
    // delay(3000);
    // servoGate.detach();

    // if gate is on undefined state let open it to release the monster
    if (closeGateState == 1 && openGateState == 1)
    {
      Serial.println("Some went wrong at initialization and let release monsters");
      moveGate(true);
    }

    Serial.println("At the end of init closeGate :");
    Serial.println(closeGateState);
    Serial.println("At the end of init openGate :");
    Serial.println(openGateState);
  }
}

void loop()
{
  DateTime_t now;
  tRtc.read_current_datetime(&now);

  // search for today ephemeride
  int tYearIndex = now.year - CURRENTYEAR;
  int tMonthIndex = now.months - 1;
  int tDayIndex = now.days - 1;

  byte sunRiseHour = pgm_read_byte(&ephemeride[tYearIndex][tMonthIndex][tDayIndex][0]);
  byte sunRiseMinute = pgm_read_byte(&ephemeride[tYearIndex][tMonthIndex][tDayIndex][1]);
  byte sunSetHour = pgm_read_byte(&ephemeride[tYearIndex][tMonthIndex][tDayIndex][2]);
  byte sunSetMinute = pgm_read_byte(&ephemeride[tYearIndex][tMonthIndex][tDayIndex][3]);

  // for testing purpose only
  testingStruct[0] = 15;
  testingStruct[1] = 33;
  testingStruct[2] = 15;
  testingStruct[3] = 35;
  if (testingStruct[0])
  {
    sunRiseHour = testingStruct[0];
    sunRiseMinute = testingStruct[1];
    sunSetHour = testingStruct[2];
    sunSetMinute = testingStruct[3];
    Serial.println("Debugging with this specific hours");
    Serial.println("sunRiseHour:");
    Serial.println(sunRiseHour);
    Serial.println("sunRiseMinute");
    Serial.println(sunRiseMinute);
    Serial.println("sunSetHour");
    Serial.println(sunSetHour);
    Serial.println("sunSetMinute");
    Serial.println(sunSetMinute);
  }

  if ((sunRiseHour == now.hours && sunRiseMinute - 1 < now.minutes) && (sunRiseHour == now.hours && sunRiseMinute + 1 > now.minutes))
  {
    Serial.println("Let go out !");
    moveGate(true);
  }
  else if ((sunSetHour == now.hours && sunSetMinute - 1 < now.minutes) && (sunSetHour == now.hours && sunSetMinute + 1 > now.minutes))
  {
    Serial.println("Bed time !");
    moveGate(false);
  }

  delay(60000 - moveDuration);
  moveDuration = 0;
}

void moveGate(bool isOpenMove)
{
  servoGate.attach(9);
  if (isOpenMove)
  {
    Serial.println("Gandalf please open this door !");
    //j'ouvre tant que pas completement ouvert
    while (digitalRead(openGatePin) != 0)
    {
      servoGate.write(65);
      delay(2000);
      moveDuration += 2;
    }
    delay(400);
    servoGate.write(95);
    servoGate.detach();
  }
  else
  {
    Serial.println("Yup close it !");
    //je ferme tant que pas completement ferme
    while (digitalRead(closeGatePin) != 0)
    {
      servoGate.write(125);
      delay(2000);
      moveDuration += 2;
    }
    delay(300);
    servoGate.write(95);
    servoGate.detach();
  }

  closeGateState = digitalRead(closeGatePin);
  openGateState = digitalRead(openGatePin);
  Serial.println("Once gate moved closeGate :");
  Serial.println(closeGateState);
  Serial.println("Once gate moved openGate :");
  Serial.println(openGateState);
}
