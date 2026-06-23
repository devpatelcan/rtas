#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SinricPro.h>
#include <SinricProTemperaturesensor.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHTC3.h>
#include <LiquidCrystal.h>
#include <math.h>

const char* WIFI_SSID    = "SSID";
const char* WIFI_PASS    = "PASS";
const char* APP_KEY      = "KEY";
const char* APP_SECRET   = "SCT";
const char* DEVICE_ID    = "ID";

SinricProTemperaturesensor& cloudSensor = SinricPro[DEVICE_ID];
Adafruit_SHTC3 shtc3;

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);


#ifndef LED_BUILTIN
#define LED_BUILTIN 2 
#endif
const unsigned long LED_BLINK_MS = 500; 
unsigned long lastLedToggleMs = 0;
bool ledState = false;

const int BUZZER_PIN = 26;
const int BUZZER_CHANNEL = 0;    
const int BUZZER_FREQ = 2000;    
const int BUZZER_RES = 8;        

const unsigned long SENSOR_READ_MS = 1000;
const unsigned long FORCE_SEND_MS  = 60000;

const float TEMP_DELTA_C  = 0.3;
const float HUMI_DELTA_PC = 2.0;

const float TEMP_WARN_HIGH = 24.0;
const float TEMP_WARN_LOW = 17.0;  
const float TEMP_HIGH = 27.0;
const float TEMP_LOW = 15.0;  


unsigned long lastReadMs  = 0;
unsigned long lastSendMs  = 0;
unsigned long lastBeepMs  = 0;

float lastTempC = NAN;
float lastHumi  = NAN;

bool readSHTC3(float &tC, float &rh) {
  sensors_event_t humidity, temp;
  if (!shtc3.getEvent(&humidity, &temp)) return false;
  tC = temp.temperature;
  rh = humidity.relative_humidity;
  return !isnan(tC) && !isnan(rh);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Start"); 

  Wire.begin(21, 22);

  if (!shtc3.begin(&Wire)) {
    Serial.println("Couldn't find SHTC3");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SHTC3 ERROR");
    while (1) { delay(10); }
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  SinricPro.onConnected([](){ Serial.println("SinricPro connected"); });
  SinricPro.onDisconnected([](){ Serial.println("SinricPro disconnected"); });
  SinricPro.begin(APP_KEY, APP_SECRET);


  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);

  delay(500); 
  lcd.clear();
}

void loop() {
  SinricPro.handle();
  unsigned long now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    if (now - lastLedToggleMs >= LED_BLINK_MS) {
      lastLedToggleMs = now;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
    }
  }

  if (now - lastReadMs >= SENSOR_READ_MS) {
    lastReadMs = now;

    float tC, rh;
    if (readSHTC3(tC, rh)) {

      if (WiFi.status() == WL_CONNECTED) {
          bool firstReading = isnan(lastTempC) || isnan(lastHumi);
          bool tempChanged = firstReading || fabs(tC - lastTempC) >= TEMP_DELTA_C;
          bool humiChanged = firstReading || fabs(rh - lastHumi)  >= HUMI_DELTA_PC;
          bool forceSend   = (now - lastSendMs) >= FORCE_SEND_MS;

          if (tempChanged || humiChanged || forceSend) {
            bool ok = cloudSensor.sendTemperatureEvent(tC, rh);
            Serial.printf("%s to Sinric: %.2f C, %.1f %%RH\n",
                          ok ? "Sent" : "Send FAILED", tC, rh);
            lastSendMs = now;
            lastTempC = tC;
            lastHumi  = rh;
          }
      }

      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(tC, 1);
      lcd.print((char)223);
      lcd.print("C   ");

      lcd.setCursor(0, 1);
      lcd.print("H:");
      lcd.print(rh, 1);
      lcd.print("%   ");

      unsigned long beepInterval = 0;

      if ((tC >= TEMP_WARN_HIGH && tC < TEMP_HIGH) || (tC <= TEMP_WARN_LOW && tC > TEMP_LOW)) {
        beepInterval = 15000; 
      } else if (tC >= TEMP_HIGH || tC <= TEMP_LOW) {
        beepInterval = 5000; 
      }

      if (beepInterval > 0 && (now - lastBeepMs >= beepInterval)) {
        ledcWrite(BUZZER_CHANNEL, 180);
        delay(150);               
        ledcWrite(BUZZER_CHANNEL, 0);

        lastBeepMs = now;
      }

    } else {
      Serial.println("SHTC3 read failed");
      lcd.setCursor(0, 0);
      lcd.print("Read FAIL");
    }
  }
}