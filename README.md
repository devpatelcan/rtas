# Room Temperature Alert System (RTAS)

<a href="https://youtu.be/mLJ8WJGYnlc">
  <img src="https://img.youtube.com/vi/mLJ8WJGYnlc/maxresdefault.jpg" width="600">
</a>


[Click here or on the image to watch the video](https://youtu.be/mLJ8WJGYnlc)

The Room Temperature Alert System (RTAS) is an IoT-based environmental monitor built with an ESP32. It actively measures room temperature and humidity, providing local visual and audio feedback, and integrates with Google Home for voice-activated readings. 

Central thermostats often fail to capture accurate temperature variations in secondary rooms, such as basements or second floors. This device acts as a standalone sensor node to monitor these specific zones and alert the user if conditions become unsafe.

## Features

* **Live Local Display:** Updates temperature and humidity readings every $1\text{s}$ on a 16x2 LCD.
* **Audio Alerts:** Sounds a passive piezo buzzer at varying intervals based on defined safety thresholds.
* **Smart Assistant Integration:** Connects to Sinric Pro via WiFi, allowing users to ask Google Home for current room readings.
* **Non-Blocking Logic:** Boots up and displays sensor data immediately, managing network connection attempts in the background.
* **Status Indication:** Uses the ESP32 onboard LED to signal WiFi state (blinking for searching, solid for connected).

## Hardware Components

| Component | Function |
| :--- | :--- |
| **ESP32-Devkit ESP-WROOM-32** | Main microcontroller handling sensor data, logic, and WiFi communication. |
| **Adafruit SHTC3** | High-precision I2C digital temperature and humidity sensor. |
| **1602 LCD Display** | Provides visual data. Wired via a 4-bit parallel interface. |
| **Passive Piezo Buzzer** | Driven by a $2\text{kHz}$ PWM signal to emit audio alerts. |
| **LM2596S Buck Converter** | Steps down the $9\text{V}$ battery input to a stable $5\text{V}$ for the MCU and LCD. |
| **10K Ohm Potentiometer** | Acts as a voltage divider to configure the contrast for the LCD. |
| **6xAA Battery Pack** | Provides the $9\text{V}$ DC power source for the device. |

## Power Distribution

Power delivery is split to handle the different logic levels required by the components:
1. The 6xAA battery pack supplies $9\text{V}$ to the LM2596S buck converter.
2. The buck converter steps the voltage down to $5\text{V}$ and feeds a power rail.
3. The $5\text{V}$ rail powers the 1602 LCD and supplies the ESP32 via its `VIN` pin.
4. The ESP32 uses its internal regulator to output $3.3\text{V}$, which powers the SHTC3 sensor and the passive buzzer.

## Firmware and Logic

The firmware is written using the Arduino framework (`<Arduino.h>`). 

### Smart Data Transmission
To respect API limits and network bandwidth, the ESP32 does not blindly send data every second. It transmits data to the Sinric Pro cloud only if the temperature/humidity delta is significant, or a hard limit of $60\text{s}$ has passed since the last update.

### Alarm Thresholds
The buzzer logic uses distinct safety zones based on current temperature readings:
* **Critical Zone ($< 15^{\circ}\text{C}$ or $> 27^{\circ}\text{C}$):** The buzzer emits a fast beep every $5\text{s}$.
* **Warning Zone ($15^{\circ}\text{C} - 17^{\circ}\text{C}$ or $24^{\circ}\text{C} - 27^{\circ}\text{C}$):** The buzzer emits a slow beep every $15\text{s}$.
* **Safe Zone ($17^{\circ}\text{C} - 24^{\circ}\text{C}$):** The alarm remains off.

## Future Improvements

* **Hardware Power Switch:** Add a dedicated switch to the chassis to cut battery power without manually disconnecting the cells.
* **Power Cycling:** Implement a programmable timer controller or utilize ESP32 deep sleep states to turn the unit off during specific hours, conserving alkaline battery life.
* **Cable Management:** Shorten the wire runs between the LM2596S buck converter and the breadboard rails to reduce parasitic inductance and electrical noise in the circuit.

## Pictures

<figure>
  <img width="645" alt="rtas2" src="https://github.com/user-attachments/assets/1b655545-fa29-4460-801b-5b88babe7a8f" />
  <figcaption><em><br>The RTAS alongside the Google Home Mini.</em></figcaption>
</figure>

<br>
<br>
<br>

<figure>
  <img width="678" alt="rtas1" src="https://github.com/user-attachments/assets/0dbdf7e9-5419-4cff-940b-2fc50e7f6268" />
  <figcaption><em><br>Internal wiring of the RTAS.</em></figcaption>
</figure>

<br>
<br>
<br>

<figure>
  <img width="440" alt="rtas 3" src="https://github.com/user-attachments/assets/f78dc924-affb-43fb-97fe-29fa33fba1b5" />
  <figcaption><em><br>RTAS displaying temperature and humidity data.</em></figcaption>
</figure>







