# IoT Smart Irrigation System

This repository contains the source code for my Bachelor's Degree project at the Faculty of Automation and Computers (UPT). The project is a complete Smart Agriculture / Smart Home system that monitors climate and soil conditions to make autonomous irrigation decisions.

## System Architecture
The project is divided into two main components:
1. **Edge Node (ESP32 - C++)**: Reads the sensors (capacitive soil moisture, DHT22, LM393 rain sensor), controls the water pump via an optically isolated relay, and transmits data via HTTP POST requests.
2. **Web Platform (.NET Blazor - C#)**: An interactive dashboard that displays environmental data in real-time and allows switching between Automatic mode and remote Manual control.

## Hardware Used
* ESP32 Microcontroller (Programmed via Arduino IDE)
* Capacitive Soil Moisture Sensor
* DHT22 Temperature and Humidity Sensor
* Rain Sensor with LM393 Comparator
* 5V Relay Module with Optocoupler + Submersible DC Pump
* 0.96" I2C OLED Display (SSD1306)

## How to Run the Project

### Web Component (.NET Blazor)
1. Open the `.sln` solution file in Visual Studio.
2. Ensure the .NET SDK is installed on your machine.
3. Run the application. Note your local IP address and the port used by the server to update the URL in the ESP32 code.

### Hardware Component (ESP32)
1. Open the `.ino` file in the Arduino IDE.
2. Update the `ssid`, `password`, and `serverUrl` variables with your local Wi-Fi credentials and the Blazor server's IP address.
3. Upload the code to the ESP32 board.