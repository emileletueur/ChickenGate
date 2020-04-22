#include <Wire.h>
#include <arduino.h>
#include "DS1307.h"

DS1307::DS1307()
{
    Wire.begin();
}

DS1307::bcd_to_decimal(byte aByte)
{
    return (aByte / 16 * 10) + (aByte % 16);
}

DS1307::decimal_to_bcd(byte aByte)
{
    return (aByte / 10 * 16) + (aByte % 10);
}

DS1307::read_current_datetime(DateTime_t *datetime)
{
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((byte)0);
    Wire.endTransmission();
    
    Wire.requestFrom(DS1307_ADDRESS, (byte)7);
    byte raw_seconds = Wire.read();
    datetime->seconds = bcd_to_decimal(raw_seconds);
    datetime->minutes = bcd_to_decimal(Wire.read());
    byte raw_hours = Wire.read();
    if (raw_hours & 64)
    { // Format 12h
        datetime->hours = bcd_to_decimal(raw_hours & 31);
        datetime->is_pm = raw_hours & 32;
    }
    else
    { // Format 24h
        datetime->hours = bcd_to_decimal(raw_hours & 63);
        datetime->is_pm = 0;
    }
    datetime->day_of_week = bcd_to_decimal(Wire.read());
    datetime->days = bcd_to_decimal(Wire.read());
    datetime->months = bcd_to_decimal(Wire.read());
    datetime->year = bcd_to_decimal(Wire.read());

    return raw_seconds & 128;
}

DS1307::adjust_current_datetime(DateTime_t *datetime)
{
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write((byte)0);
    Wire.write(decimal_to_bcd(datetime->seconds) & 127);
    Wire.write(decimal_to_bcd(datetime->minutes));
    Wire.write(decimal_to_bcd(datetime->hours) & 63);
    Wire.write(decimal_to_bcd(datetime->day_of_week));
    Wire.write(decimal_to_bcd(datetime->days));
    Wire.write(decimal_to_bcd(datetime->months));
    Wire.write(decimal_to_bcd(datetime->year));
    Wire.endTransmission();
}