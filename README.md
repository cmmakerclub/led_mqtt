# led_mqtt

This project use [MQTT-Connector](https://github.com/cmmakerclub/MQTT-Connector) and [adafruit neopixel](https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-installation).


## Setup


Set DEVICE\_NAME, WIFI\_SSID, WIFI\_PASSWORD And MQTT SERVER.


## MQTT API

### Topic $/set_mode

eg. WORKSHOP/LED_02/$/set\_led

Payload

	idle
	run
	rainbow
	rainbow2
	rainbow3

Example

	run
	
	
idle

	Do nothing.
		
run

	led running
	
rainbow
	
	show rainbow for shot of time
	
rainbow2
	
	show rainbow for shot of time	
	
rainbow3
	
	show rainbow for shot of time	

---

### Topic $/set_led

eg. WORKSHOP/LED_02/$/set\_led\_all

Payload

	FFAA33
	
	FF = Red
	AA = Green
	33 = Blue

Example

	FF00AA

---

### Topic $/set_led

eg. WORKSHOP/LED_02/$/set\_led


Set color for specific led.

Payload

	00FFAA33
	
	xx = led number
	FF = Red
	AA = Green
	33 = Blue

Example

	01FF00AA
	-> set led number 1 with color FF00AA
	
	02FF00AA,03FA00AF,041F00AA
	-> set led number 2 with color FF00AA
	-> set led number 3 with color FA00AF
	-> set led number 4 with color 1F00AA

---

### Topic $/set\_run\_length

eg. WORKSHOP/LED_02/$/set\_run\_length


In **run mode** set length of trail. (1 - max led length)
	
Example	
	
	40

---

### Topic $/set\_run\_led\_main

eg. WORKSHOP/LED_02/$/set\_run\_led\_main


In **run mode** set running main led color.

Example

	FF00AA
	-> set color for main running led

---

### Topic $/set\_run\_led\_trail

eg. WORKSHOP/LED_02/$/set\_run\_led\_trail


In **run mode** set running led trail color.

Example

	FF00AA
	-> set color for main running led

---

### Topic $/set\_run\_delay

eg. WORKSHOP/LED_02/$/set\_run\_delay


In **run mode** set delay for running. (0 - 10000)

Example

	30

---
