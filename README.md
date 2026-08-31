# ESP32 Smart Home LED Control

A university smart home prototype based on the **ESP32** for remotely controlling four LEDs through a Wi-Fi web interface.

The project combines an ESP32, four LEDs, current-limiting resistors, a physical prototype model, and an embedded web server to provide wireless control of the lighting system.

## Features

- ESP32-based embedded system
- Wi-Fi Access Point mode
- Embedded HTTP web server
- Control of four LEDs
- Web-based LED status monitoring
- Individual LED toggle control
- Automatic status refresh
- Physical smart home prototype
- Arduino-based firmware

## System Overview

The ESP32 creates its own Wi-Fi network and runs an embedded web server.

After connecting a phone to the ESP32 Wi-Fi network, the user can open the web interface and monitor the state of the four LEDs. Each LED can be individually switched between ON and OFF states.

## Hardware

| Component | Quantity | Function |
|---|---:|---|
| ESP32 | 1 | Main controller and Wi-Fi communication |
| LED | 4 | Simulated room lighting |
| 220 Ω Resistor | 4 | LED current limiting |
| 5 V Lithium Battery / Power Bank | 1 | Power supply |
| Perfboard | 1 | Circuit assembly |
| Building Model | 1 | Smart home prototype |
| Power Switch | 1 | Main power control |

## Web Interface

The embedded web interface provides:

- Current status of all four LEDs
- Individual ON/OFF control
- Automatic status refresh
- Connection status indication

The web application is embedded directly into the ESP32 firmware.

## Firmware

The firmware was developed using **Arduino/C++** with the following libraries:

- `WiFi.h`
- `WebServer.h`

The ESP32 operates in **Access Point (AP) mode** and hosts the control interface locally.

### Main Web Endpoints

| Endpoint | Function |
|---|---|
| `/` | Serves the web control interface |
| `/toggle` | Toggles an individual LED |
| `/status` | Returns the current LED states as JSON |

## LED Control

Four GPIO pins are assigned to the LEDs:

```cpp
const int LED_PINS[4] = {25, 26, 27, 14};
