#include <Arduino.h>
#include <MqttConnector.h>

extern MqttConnector* mqtt;

extern String MQTT_CLIENT_ID;
extern String MQTT_PREFIX;

extern int relayPin;
extern int relayPinState;
extern char myName[];

extern int LED_PIN;

extern int maxLed;

extern String ledData[];

extern String ledMode;

extern int ledDelay;
extern int ledFadeLength;

extern String LED_MODE_IDLE;
extern String LED_MODE_RUN;
extern String LED_MODE_CENTER;
extern String LED_MODE_RAINBOW;
extern String LED_MODE_RAINBOW_2;
extern String LED_MODE_RAINBOW_3;

extern String mainLedRunData;
extern String trailLedRunData;

extern Adafruit_NeoPixel strip;

void register_receive_hooks() {
  mqtt->on_subscribe([&](MQTT::Subscribe *sub) -> void {
    Serial.printf("myName = %s \r\n", myName);
    sub->add_topic(MQTT_PREFIX + myName + "/$/+");
    sub->add_topic(MQTT_PREFIX + MQTT_CLIENT_ID + "/$/+");
  });

  mqtt->on_before_message_arrived_once([&](void) { });

  mqtt->on_message([&](const MQTT::Publish & pub) { });

  mqtt->on_after_message_arrived([&](String topic, String cmd, String payload) {
    Serial.printf("topic: %s\r\n", topic.c_str());
    Serial.printf("cmd: %s\r\n", cmd.c_str());
    Serial.printf("payload: %s\r\n", payload.c_str());

    if (cmd == "$/set_run_delay")
    {
      if (payload.toInt() >= 0 && payload.toInt() < 10000)
      {
        ledDelay = payload.toInt();
      }
    }
    if (cmd == "$/set_run_led_main")
    {
      mainLedRunData = payload;
    }
    if (cmd == "$/set_run_led_trail")
    {
      trailLedRunData = payload;
    }
    if (cmd == "$/set_run_length")
    {
      if (payload.toInt() > 0 && payload.toInt() < maxLed)
      {
        ledDelay = payload.toInt();
      }
      
      ledFadeLength = payload.toInt();
    }
    
    if (cmd == "$/set_mode")
    {
      if (payload == LED_MODE_IDLE)
      { 
        ledMode = LED_MODE_IDLE;
      }
      else if (payload == LED_MODE_RUN)
      {
        ledMode = LED_MODE_RUN;
      }
      else if (payload == LED_MODE_CENTER)
      {
        ledMode = LED_MODE_CENTER;
      }
      else if (payload == LED_MODE_RAINBOW)
      {
        ledMode = LED_MODE_RAINBOW;
      }
      else if (payload == LED_MODE_RAINBOW_2)
      {
        ledMode = LED_MODE_RAINBOW_2;
      }
      else if (payload == LED_MODE_RAINBOW_3)
      {
        ledMode = LED_MODE_RAINBOW_3;
      }

    }

    if (cmd == "$/set_led")
    {
        String ledNumber = payload.substring(0, 2);
        String rString = payload.substring(2, 4);
        String gString = payload.substring(4, 6);
        String bString = payload.substring(6, 8);
        
        char tempChar[sizeof(2)];

        while (payload.length() >= 8)
        {
          if (payload[0] == ',')
          {
            payload.remove(0, 1);
          }

          ledNumber = payload.substring(0, 2);
          rString = payload.substring(2, 4);
          gString = payload.substring(4, 6);
          bString = payload.substring(6, 8);          
          
          ledNumber.toCharArray(tempChar, sizeof(tempChar));
          unsigned int led = strtoul(tempChar, NULL, 16);
          
          rString.toCharArray(tempChar, sizeof(tempChar));
          unsigned int r = strtoul(tempChar, NULL, 16);
          
          gString.toCharArray(tempChar, sizeof(tempChar));
          unsigned int g = strtoul(tempChar, NULL, 16);
          
          bString.toCharArray(tempChar, sizeof(tempChar));
          unsigned int b = strtoul(tempChar, NULL, 16);          

          ledData[led] = payload.substring(0, 8);

          strip.setPixelColor(led, strip.Color(r, g, b));

          payload.remove(0, 8);
        }
        strip.show();
    }
    
    if (cmd == "$/command") {
      if (payload == "ON") {
        digitalWrite(relayPin, HIGH);
        digitalWrite(LED_PIN, LOW);
        relayPinState = HIGH;
      }
      else if (payload == "OFF") {
        digitalWrite(relayPin, LOW);
        digitalWrite(LED_PIN, HIGH);
        relayPinState = LOW;
      }
    }
    else if (cmd == "$/reboot") {
      ESP.restart();
    }
    else {
      // another message.
    }
  });
}
