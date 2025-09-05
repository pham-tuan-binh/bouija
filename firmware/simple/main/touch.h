#ifndef TOUCH_H
#define TOUCH_H

#include "esp_err.h"
#include "driver/touch_sensor.h"
#include "driver/touch_sensor_common.h"

// Touch input configuration
#define TOUCH_GPIO_SLAP 4
#define TOUCH_GPIO_CAP 5
#define TOUCH_GPIO_SUP 7
#define TOUCH_GPIO_PEACE 6
#define TOUCH_THRESHOLD 70000

// Button indices
typedef enum
{
    BUTTON_SLAP = 0,
    BUTTON_CAP = 1,
    BUTTON_SUP = 2,
    BUTTON_PEACE = 3,
    BUTTON_NONE = -1
} button_t;

// Function declarations
esp_err_t touch_init(void);
esp_err_t touch_deinit(void);
bool touch_is_pressed(int touch_pin);
button_t touch_get_pressed_button(void);
const char *touch_get_button_name(button_t button);

// Debug functions
void touch_debug_monitor(void);

#endif // TOUCH_H
