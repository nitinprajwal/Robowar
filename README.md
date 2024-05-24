# Car Control System with ESP32

This code provides a web interface for controlling a car system using an ESP32 microcontroller. It allows you to control the steering, speed, and a stepper motor through a web interface hosted on the ESP32.

## Features

- Control the steering servo using a slider
- Control the speed of the car using an ESC (Electronic Speed Control) slider
- Control the speed of a stepper motor using a vertical slider
- Real-time feedback on the web interface for steering angle and speed values

## Requirements

- ESP32 development board
- Servo motor for steering
- Electronic Speed Control (ESC) for controlling the car's speed
- Stepper motor
- WiFi network credentials

## Installation

1. Install the required Arduino libraries:
   - `AsyncTCP.h`
   - `ESPAsyncWebServer.h`
   - `AccelStepper.h`

2. In the Arduino IDE, go to `File > Preferences` and add the following URLs in the "Additional Boards Manager URLs" field:
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - `https://dl.espressif.com/dl/package_esp32_index.json`

3. After adding the URLs, go to `Tools > Board > Boards Manager` and install the "ESP32" board by Espressif Systems.

4. **Installing the ServoESP32 Library**

   The ServoESP32 Library makes it easier to control a servo motor with your ESP32 using the Arduino IDE. Follow the next steps to install the library in the Arduino IDE:

   1. Go to `Sketch > Include Library > Manage Libraries...`
   2. Search for **ServoESP32**.
   3. **Select version 1.0.3** (at the moment there are some issues with version 1.1.0, and 1.1.1).
   4. Install the library.

5. Connect the servo motor for steering to GPIO pin 13 (`servoPin`).
6. Connect the ESC signal wire to GPIO pin 15 (`signalOut`).
7. Connect the stepper motor direction pin to GPIO pin 25 (`dirPin`).
8. Connect the stepper motor step pin to GPIO pin 26 (`stepPin`).
9. Update the `ssid` and `password` constants with your WiFi network credentials.
10. Upload the code to your ESP32 board using the Arduino IDE or your preferred method.

## Usage

1. After uploading the code, the ESP32 will create a WiFi access point with the specified SSID and password.
2. Connect to the ESP32's WiFi network from your computer or mobile device.
3. Open a web browser and navigate to the IP address displayed in the Serial Monitor (usually `192.168.4.1`).
4. The web interface will load, displaying sliders for controlling the steering, speed, and stepper motor.
5. Adjust the sliders to control the respective components in real-time.

## Web Interface

The web interface consists of the following components:

- **Steering Control**: A slider to control the steering angle of the servo motor. The current rotation angle is displayed above the slider.
- **ESC Control**: A slider to control the speed of the car by adjusting the ESC value.
- **Stepper Motor Control**: A vertical slider to control the speed of the stepper motor.