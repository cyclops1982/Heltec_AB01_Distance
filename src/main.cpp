#include <Arduino.h>
// #include <LoRaWanMinimal_APP.h>
//#include "LoraWan_config.h"
#include <Wire.h>
#include <VL53L0X.h>

#define VERSION "0.1"

static TimerEvent_t sleepTimer;
bool lowpower = false;

VL53L0X sensor;

static uint16_t getBatteryVoltageFloat(void)
{
  float temp = 0;
  pinMode(VBAT_ADC_CTL, OUTPUT);
  digitalWrite(VBAT_ADC_CTL, LOW);

  delay(50);

  for (int i = 0; i < 50; i++)
  { // read 50 times and get average
    temp += analogReadmV(ADC);
  }
  pinMode(VBAT_ADC_CTL, INPUT);
  return (uint16_t)((temp / 50) * 2);
}

void onWakeUp()
{
  Serial.printf("Woke up!\n");
  TimerStop(&sleepTimer);
  lowpower = false;
}

uint16_t getMeasurement()
{
  uint16_t distance = 0;

  digitalWrite(Vext, LOW);
  delay(50);
  Wire.begin();
  delay(50);

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    distance = 0;
  }
  else
  {

    // lower the return signal rate limit (default is 0.25 MCPS)
    sensor.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

    // Set high accuracy
    sensor.setMeasurementTimingBudget(200000);

    distance = sensor.readRangeSingleMillimeters();
    if (sensor.timeoutOccurred())
    {
      Serial.println("Timeout occured while reading sensor.");
      distance = 0;
    }
  }

  Wire.end();
  digitalWrite(Vext, HIGH);
  delay(100);
  return distance;
}

void setup()
{
  pinMode(Vext, OUTPUT);
  Serial.begin(115200);
  delay(5000);
  Serial.printf("Booting version: %s\n", VERSION);

  TimerInit(&sleepTimer, onWakeUp);

  /*
  LoRaWAN.begin(loraWanClass, loraWanRegion);
  LoRaWAN.setAdaptiveDR(false);
  LoRaWAN.setFixedDR(5);

  while (1)
  {
    Serial.print("Joining... ");
    LoRaWAN.joinOTAA(appEui, appKey);
    if (!LoRaWAN.isJoined())
    {
      // In this example we just loop until we're joined, but you could
      // also go and start doing other things and try again later
      Serial.println("JOIN FAILED! Sleeping for 5 seconds");
      delay(5000);
    }
    else
    {
      Serial.println("JOINED");
      break;
    }
  }*/
}

void loop()
{
  if (lowpower)
  {
    lowPowerHandler();
  }
  else
  {
    uint16_t distance = getMeasurement();
    uint16_t batteryVoltage = getBatteryVoltageFloat();
    delay(100);

    digitalWrite(Vext, HIGH);

    Serial.println();
    delay(100);
    TimerSetValue(&sleepTimer, 10000);
    TimerStart(&sleepTimer);
    lowpower = true;
  }
}