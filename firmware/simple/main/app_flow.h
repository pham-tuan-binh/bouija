#ifndef APP_FLOW_H
#define APP_FLOW_H

#include "esp_err.h"
#include "touch.h"

// Application states
typedef enum {
    APP_STATE_INIT,
    APP_STATE_LOADING,
    APP_STATE_BUTTON_SHIMMER,
    APP_STATE_BUTTON_PRESSED,
    APP_STATE_SHOWING_MESSAGE,
    APP_STATE_RETURN_TO_BUTTONS
} app_state_t;

// Function declarations
esp_err_t app_flow_init(void);
void app_flow_run(void);
app_state_t app_flow_get_current_state(void);
const char* app_flow_get_state_name(app_state_t state);

#endif // APP_FLOW_H
