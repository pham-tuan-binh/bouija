# ESP32 LED Board Application

A simple interactive LED board application for ESP32 with 40 LEDs and 4 capacitive touch buttons.

## Hardware Configuration

- **40 LEDs**: First 36 LEDs represent alphabet (A-Z, 0-9), last 4 LEDs are for buttons
- **4 Capacitive Touch Buttons**: SLAP, CAP, SUP, PEACE
- **LED Strip**: WS2812B on GPIO 8
- **Touch Pins**:
  - SLAP: GPIO 4
  - CAP: GPIO 5
  - SUP: GPIO 6
  - PEACE: GPIO 7

## Application Flow

1. **Loading Sequence**: Rainbow color sequence lights up LEDs 0-35 (alphabet LEDs) in sequence
2. **Button Highlighting**: The 4 button LEDs (36-39) light up with different colors:
   - SLAP: Red (LED 39)
   - CAP: Cyan (LED 38)
   - SUP: Yellow (LED 37)
   - PEACE: Green (LED 36)
3. **Button Interaction**: When a button is pressed:
   - All LEDs clear
   - Only the pressed button LED lights up
   - A message is displayed on the alphabet LEDs
   - Returns to button highlighting after 2 seconds

## Button Messages

- **SLAP**: "SLAP"
- **CAP**: "CAP"
- **SUP**: "WASSUP THIS IS BINH"
- **PEACE**: "PEACE"

## Setup

This requires the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation) toolchain to be installed.

```bash
idf.py build
idf.py -p /dev/{DEVICE_PORT} flash
```

## Features

- Rainbow loading animation
- Capacitive touch detection
- Text display on alphabet LEDs
- Interactive button responses
- Brightness control
- HSV to RGB color conversion for smooth rainbow effects

## Technical Details

- **Modular Architecture**: Clean separation of concerns with dedicated modules for LED control, touch sensing, and application logic
- **State Machine**: Application flow uses a state machine for predictable behavior
- **Hardware Abstraction**: LED and touch modules provide clean hardware abstraction layers
- Uses ESP-IDF touch pad driver for capacitive sensing
- LED strip driver with RMT backend for precise timing
- FreeRTOS tasks for non-blocking operation
- Configurable brightness and touch thresholds

## Code Architecture

The application follows best practices for embedded software development:

- **led.h/led.c**: LED hardware abstraction layer
- **touch.h/touch.c**: Touch sensor module
- **app_flow.h/app_flow.c**: Application logic and state machine
- **main.c**: Application entry point and initialization

See `main/ARCHITECTURE.md` for detailed architecture documentation.
