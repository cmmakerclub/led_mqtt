#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef ESP8266 
  #include <ESP8266WiFi.h>
#else 
  #include <WiFi.h>
#endif

#include <ArduinoJson.h>
#include <MqttConnector.h>
#include <Wire.h>
#include <SPI.h>

#include "init_mqtt.h"
#include "_publish.h"
#include "_receive.h"
#include "_config.h"

#define PIN 23
#define MAX_LED 150

String LED_MODE_IDLE = "idle";
String LED_MODE_RUN = "run";
String LED_MODE_RAINBOW = "rainbow";
String LED_MODE_RAINBOW_2 = "rainbow2";
String LED_MODE_RAINBOW_3 = "rainbow3";
String LED_MODE_CENTER = "center";

String LED_TRIGGER_FIRER = "firer";

MqttConnector *mqtt; 

int maxLed = MAX_LED; 
int relayPin = 15; 
int relayPinState = HIGH;
int LED_PIN = 2;
int ledDelay = 30;
int ledFadeLength = 30;
int ledRunLead = 0;

float ledRunAlpha[MAX_LED];
String ledData[MAX_LED];

String ledMode = LED_TRIGGER_FIRER;
String triggerMode = "";

///////////////////
/// LED RUN //////
/////////////////

unsigned int rMainRun;
unsigned int gMainRun;
unsigned int bMainRun;

unsigned int rTrailRun;
unsigned int gTrailRun;
unsigned int bTrailRun;

unsigned int r;
unsigned int g;
unsigned int b;

String mainLedRunData = "FFFFFF";
String trailLedRunData = "000000";

String rString;
String gString;
String bString;

char tempChar[sizeof(2)];

//////////////////

Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_LED, PIN, NEO_GRB + NEO_KHZ800);

char myName[40];

void init_hardware()
{
  pinMode(relayPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(relayPin, relayPinState);;
  // serial port initialization
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Starting...");
}

void init_wifi() {
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(50);
  const char* ssid =  WIFI_SSID.c_str();
  const char* pass =  WIFI_PASSWORD.c_str();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", ssid, pass);
    delay(300);
  }
  Serial.println("WiFi Connected.");
  digitalWrite(2, HIGH);
}

void init_led() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
}

void setup()
{ 
  init_hardware();
  init_wifi();
  init_mqtt();
  init_led();
}

void loop()
{
  if (triggerMode == LED_TRIGGER_FIRER)
  {
    int fireLead = 0;

    for (int z = 0; z < maxLed + ledFadeLength * 3; z++)
    {
      //MAX_LED
      if (mainLedRunData != "")
      {
        rString = mainLedRunData.substring(0, 2);
        gString = mainLedRunData.substring(2, 4);
        bString = mainLedRunData.substring(4, 6);          
        
        rString.toCharArray(tempChar, sizeof(tempChar));
        rMainRun = strtoul(tempChar, NULL, 16);
        
        gString.toCharArray(tempChar, sizeof(tempChar));
        gMainRun  = strtoul(tempChar, NULL, 16);
        
        bString.toCharArray(tempChar, sizeof(tempChar));
        bMainRun  = strtoul(tempChar, NULL, 16);

        rString = trailLedRunData.substring(0, 2);
        gString = trailLedRunData.substring(2, 4);
        bString = trailLedRunData.substring(4, 6);          
        
        rString.toCharArray(tempChar, sizeof(tempChar));
        rTrailRun = strtoul(tempChar, NULL, 16);
        
        gString.toCharArray(tempChar, sizeof(tempChar));
        gTrailRun  = strtoul(tempChar, NULL, 16);
        
        bString.toCharArray(tempChar, sizeof(tempChar));
        bTrailRun  = strtoul(tempChar, NULL, 16);
      }

      if (fireLead < MAX_LED)
      {
        strip.setPixelColor(fireLead, strip.Color(r, g, b));
      }

      int backwardLed = fireLead;
      int useR;
      int useG;
      int useB;
      float percentBrightness = 1;
      for (int i = 0; i < ledFadeLength; i++)
      { 
        backwardLed--;

        if (backwardLed < 0)
        {
          backwardLed = MAX_LED - 1;
        }

        int UseMax = 0;

        if (backwardLed >= MAX_LED)
        {
          UseMax = MAX_LED - 1;
        }
        else 
        {
          UseMax = backwardLed;
        }

        // try get main led color
        if (ledData[UseMax] == "")
        {
          r = rMainRun;
          g = gMainRun;
          b = bMainRun;
        }
        else
        {
          rString = ledData[UseMax].substring(2, 4);
          gString = ledData[UseMax].substring(4, 6);
          bString = ledData[UseMax].substring(6, 8);

          rString.toCharArray(tempChar, sizeof(tempChar));
          r = strtoul(tempChar, NULL, 16);
          
          gString.toCharArray(tempChar, sizeof(tempChar));
          g = strtoul(tempChar, NULL, 16);
          
          bString.toCharArray(tempChar, sizeof(tempChar));
          b = strtoul(tempChar, NULL, 16);

          if (r + g + b < 20)
          {
            // do not show very low light
            r = rMainRun;
            g = gMainRun;
            b = bMainRun;
          }
        }

        //(1 - alpha) * RGB_background.r + alpha * RGBA_color.r,
        //(1 - alpha) * RGB_background.g + alpha * RGBA_color.g,
        //(1 - alpha) * RGB_background.b + alpha * RGBA_color.b
        
        if (i == ledFadeLength - 1)
        {
          percentBrightness = 0;
        }
        
        useR = (float)(1 - percentBrightness) * rTrailRun + percentBrightness * r;
        useG = (float)(1 - percentBrightness) * gTrailRun + percentBrightness * g;
        useB = (float)(1 - percentBrightness) * bTrailRun + percentBrightness * b;

        percentBrightness = (float)((float)(ledFadeLength - i) / ledFadeLength);
        
        if (backwardLed < MAX_LED)
        {
          strip.setPixelColor(backwardLed, strip.Color(useR, useG, useB));
        }
        else 
        {
          strip.setPixelColor(MAX_LED - 1, strip.Color(useR, useG, useB));          
        }

      }
      
      delay(ledDelay);
      strip.show();
      fireLead++;
    }
    triggerMode = "";
  }

  if (ledMode == LED_MODE_IDLE)
  {
    
  }
  else if (ledMode == LED_MODE_RUN)
  {
    //MAX_LED
    
    if (mainLedRunData != "")
    {
      rString = mainLedRunData.substring(0, 2);
      gString = mainLedRunData.substring(2, 4);
      bString = mainLedRunData.substring(4, 6);          
      
      rString.toCharArray(tempChar, sizeof(tempChar));
      rMainRun = strtoul(tempChar, NULL, 16);
      
      gString.toCharArray(tempChar, sizeof(tempChar));
      gMainRun  = strtoul(tempChar, NULL, 16);
      
      bString.toCharArray(tempChar, sizeof(tempChar));
      bMainRun  = strtoul(tempChar, NULL, 16);

      rString = trailLedRunData.substring(0, 2);
      gString = trailLedRunData.substring(2, 4);
      bString = trailLedRunData.substring(4, 6);          
      
      rString.toCharArray(tempChar, sizeof(tempChar));
      rTrailRun = strtoul(tempChar, NULL, 16);
      
      gString.toCharArray(tempChar, sizeof(tempChar));
      gTrailRun  = strtoul(tempChar, NULL, 16);
      
      bString.toCharArray(tempChar, sizeof(tempChar));
      bTrailRun  = strtoul(tempChar, NULL, 16);
    }

    strip.setPixelColor(ledRunLead, strip.Color(r, g, b));

    int backwardLed = ledRunLead;
    int useR;
    int useG;
    int useB;
    float percentBrightness = 1;
    for (int i = 0; i < ledFadeLength; i++)
    { 
      backwardLed--;

      if (backwardLed < 0)
      {
        backwardLed = MAX_LED - 1;
      }

      if (backwardLed >= MAX_LED)
      {
        backwardLed = MAX_LED - 1;
      }

      // try get main led color
      if (ledData[backwardLed] == "")
      {
        r = rMainRun;
        g = gMainRun;
        b = bMainRun;
      }
      else
      {
        rString = ledData[backwardLed].substring(2, 4);
        gString = ledData[backwardLed].substring(4, 6);
        bString = ledData[backwardLed].substring(6, 8);          
        
        rString.toCharArray(tempChar, sizeof(tempChar));
        r = strtoul(tempChar, NULL, 16);
        
        gString.toCharArray(tempChar, sizeof(tempChar));
        g = strtoul(tempChar, NULL, 16);
        
        bString.toCharArray(tempChar, sizeof(tempChar));
        b = strtoul(tempChar, NULL, 16);

        if (r + g + b < 20)
        {
          // do not show very low light
          r = rMainRun;
          g = gMainRun;
          b = bMainRun;
        }
      }

      //(1 - alpha) * RGB_background.r + alpha * RGBA_color.r,
      //(1 - alpha) * RGB_background.g + alpha * RGBA_color.g,
      //(1 - alpha) * RGB_background.b + alpha * RGBA_color.b
      
      if (i == ledFadeLength - 1)
      {
        percentBrightness = 0;
      }
      
      useR = (float)(1 - percentBrightness) * rTrailRun + percentBrightness * r;
      useG = (float)(1 - percentBrightness) * gTrailRun + percentBrightness * g;
      useB = (float)(1 - percentBrightness) * bTrailRun + percentBrightness * b;

      percentBrightness = (float)((float)(ledFadeLength - i) / ledFadeLength);
      
      if (backwardLed > maxLed)
      {
        strip.setPixelColor(maxLed - 1, strip.Color(useR, useG, useB));
      }
      else
      {
        strip.setPixelColor(backwardLed, strip.Color(useR, useG, useB));
      }
    }
    
    delay(ledDelay);
    strip.show();
    ledRunLead++;

    if (ledRunLead > MAX_LED)
    {
      ledRunLead = 0;
    }

  }
  else if (ledMode == LED_MODE_CENTER)
  {
  
  }
  else if (ledMode == LED_MODE_RAINBOW)
  {
    rainbow(20);
  }
  else if (ledMode == LED_MODE_RAINBOW_2)
  {
    rainbowCycle(20);
  }
  else if (ledMode == LED_MODE_RAINBOW_3)
  {
    theaterChaseRainbow(20);
  }
  
  mqtt->loop();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
