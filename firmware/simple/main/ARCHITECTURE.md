# Code Architecture

This document describes the modular architecture of the ESP32 LED Board Application.

## Module Structure

### 1. **main.c** - Application Entry Point

- **Responsibility**: Application initialization and startup
- **Dependencies**: led.h, app_flow.h
- **Functions**:
  - `app_main()`: Initializes LED strip, sets brightness, and starts application flow

### 2. **led.h/led.c** - LED Hardware Abstraction Layer

- **Responsibility**: LED strip control and animations
- **Dependencies**: ESP-IDF LED strip driver, FreeRTOS
- **Key Functions**:
  - `led_init()`: Initialize LED strip hardware
  - `led_set_led()`: Set individual LED color
  - `led_clear_all()`: Turn off all LEDs
  - `led_show_loading_sequence()`: Rainbow loading animation
  - `led_highlight_buttons()`: Highlight button LEDs
  - `led_show_text_sequence()`: Display text on alphabet LEDs
  - `led_hsv_to_rgb()`: Color space conversion

### 3. **touch.h/touch.c** - Touch Sensor Module

- **Responsibility**: Capacitive touch button detection
- **Dependencies**: ESP-IDF touch pad driver
- **Key Functions**:
  - `touch_init()`: Initialize touch sensors
  - `touch_get_pressed_button()`: Get currently pressed button
  - `touch_is_pressed()`: Check if specific touch pin is pressed
  - `touch_get_button_name()`: Get button name string

### 4. **app_flow.h/app_flow.c** - Application Logic Controller

- **Responsibility**: Application state machine and user interaction flow
- **Dependencies**: led.h, touch.h, FreeRTOS
- **Key Functions**:
  - `app_flow_init()`: Initialize application state machine
  - `app_flow_run()`: Main application loop with state machine
  - `app_flow_get_current_state()`: Get current application state

## Design Principles

### Separation of Concerns

- **Hardware Layer**: LED and touch modules handle only hardware-specific operations
- **Application Layer**: app_flow module handles business logic and state management
- **Entry Point**: main.c handles initialization and startup

### Single Responsibility Principle

- Each module has a single, well-defined responsibility
- LED module only handles LED operations
- Touch module only handles touch detection
- App flow module only handles application logic

### Dependency Inversion

- High-level modules (app_flow) depend on abstractions (interfaces)
- Low-level modules (led, touch) implement specific hardware functionality
- Main module orchestrates the initialization

### State Machine Pattern

- Application flow uses a clear state machine for predictable behavior
- States: INIT → LOADING → BUTTON_HIGHLIGHT → BUTTON_PRESSED → SHOWING_MESSAGE → RETURN_TO_BUTTONS

## Data Flow

```
main.c
  ↓ (initializes)
led.c ← app_flow.c → touch.c
  ↓ (controls)      ↓ (detects)
LED Strip        Touch Pins
```

## Benefits of This Architecture

1. **Maintainability**: Each module can be modified independently
2. **Testability**: Modules can be unit tested in isolation
3. **Reusability**: LED and touch modules can be reused in other projects
4. **Readability**: Clear separation makes code easier to understand
5. **Scalability**: Easy to add new features or modify existing ones
