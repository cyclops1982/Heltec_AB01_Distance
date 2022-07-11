#include <Arduino.h>
// #include <LoRaWanMinimal_APP.h>
//#include "LoraWan_config.h"
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

#define VERSION "0.1"

static TimerEvent_t sleepTimer;
bool lowpower = false;

Adafruit_VL53L0X sensor = Adafruit_VL53L0X();

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
    digitalWrite(Vext, LOW);
    delay(50);

    if (!sensor.begin())
    {
      Serial.println(F("Failed to boot VL53L0X"));
      digitalWrite(Vext, HIGH);
      delay(1000);
      return;
    }

    Serial.print("Reading a measurement... ");
    VL53L0X_RangingMeasurementData_t measure;
    sensor.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
    Wire.end();

    // Vext OFF
    digitalWrite(Vext, HIGH);

    if (measure.RangeStatus != 4)
    { // phase failures have incorrect data
      Serial.print("Distance (mm): ");
      Serial.println(measure.RangeMilliMeter);
    }
    else
    {
      Serial.println(" out of range ");
    }

    digitalWrite(Vext, HIGH);

    Serial.println();
    delay(100);
    TimerSetValue(&sleepTimer, 30000);
    TimerStart(&sleepTimer);
    lowpower = true;
  }
}