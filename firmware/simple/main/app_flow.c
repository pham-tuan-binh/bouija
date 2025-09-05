#include "app_flow.h"
#include "led.h"
#include "touch.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "APP_FLOW";

// Application state
static app_state_t current_state = APP_STATE_INIT;
static button_t last_pressed_button = BUTTON_NONE;

// Button messages
static const char* button_messages[] = {
    "SLAP",                    // BUTTON_SLAP
    "CAP",                     // BUTTON_CAP
    "WASSUP THIS IS BINH",     // BUTTON_SUP
    "PEACE"                    // BUTTON_PEACE
};

// Button LED indices (last 4 LEDs)
static const int button_leds[] = {39, 38, 37, 36}; // SLAP, CAP, SUP, PEACE

// Button colors
static const uint32_t button_colors[] = {
    LED_COLOR_RED,    // SLAP
    LED_COLOR_CYAN,   // CAP
    LED_COLOR_YELLOW, // SUP
    LED_COLOR_GREEN   // PEACE
};

esp_err_t app_flow_init(void)
{
    ESP_LOGI(TAG, "Initializing application flow...");
    
    // Initialize touch sensors
    esp_err_t ret = touch_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize touch sensors: %s", esp_err_to_name(ret));
        return ret;
    }
    
    current_state = APP_STATE_LOADING;
    ESP_LOGI(TAG, "Application flow initialized successfully");
    
    return ESP_OK;
}

void app_flow_run(void)
{
    ESP_LOGI(TAG, "Starting application flow...");
    
    // Debug: Show initial touch values
    touch_debug_monitor();
    
    // Debug counter for periodic monitoring
    int debug_counter = 0;
    
    while (1) {
        switch (current_state) {
            case APP_STATE_LOADING:
                ESP_LOGI(TAG, "State: LOADING");
                led_show_loading_sequence();
                current_state = APP_STATE_BUTTON_HIGHLIGHT;
                break;
                
            case APP_STATE_BUTTON_HIGHLIGHT:
                ESP_LOGI(TAG, "State: BUTTON_HIGHLIGHT");
                led_highlight_buttons();
                current_state = APP_STATE_BUTTON_PRESSED;
                break;
                
            case APP_STATE_BUTTON_PRESSED:
                {
                    button_t pressed_button = touch_get_pressed_button();
                    if (pressed_button != BUTTON_NONE) {
                        ESP_LOGI(TAG, "Button %s pressed!", touch_get_button_name(pressed_button));
                        last_pressed_button = pressed_button;
                        current_state = APP_STATE_SHOWING_MESSAGE;
                    }
                }
                break;
                
            case APP_STATE_SHOWING_MESSAGE:
                ESP_LOGI(TAG, "State: SHOWING_MESSAGE");
                
                // Clear all LEDs and highlight only the pressed button
                led_clear_all();
                vTaskDelay(pdMS_TO_TICKS(100));
                
                led_set_led(button_leds[last_pressed_button], button_colors[last_pressed_button]);
                vTaskDelay(pdMS_TO_TICKS(500));
                
                // Show the message
                led_show_text_sequence(button_messages[last_pressed_button], LED_COLOR_WHITE);
                
                // Wait a bit then return to button highlighting
                vTaskDelay(pdMS_TO_TICKS(2000));
                current_state = APP_STATE_RETURN_TO_BUTTONS;
                break;
                
            case APP_STATE_RETURN_TO_BUTTONS:
                ESP_LOGI(TAG, "State: RETURN_TO_BUTTONS");
                current_state = APP_STATE_BUTTON_HIGHLIGHT;
                break;
                
            default:
                ESP_LOGE(TAG, "Unknown state: %d", current_state);
                current_state = APP_STATE_LOADING;
                break;
        }
        
        // Debug: Show touch values every 100 iterations (about every 5 seconds)
        debug_counter++;
        if (debug_counter >= 100) {
            touch_debug_monitor();
            debug_counter = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // Small delay to prevent busy waiting
    }
}

app_state_t app_flow_get_current_state(void)
{
    return current_state;
}

const char* app_flow_get_state_name(app_state_t state)
{
    switch (state) {
        case APP_STATE_INIT: return "INIT";
        case APP_STATE_LOADING: return "LOADING";
        case APP_STATE_BUTTON_HIGHLIGHT: return "BUTTON_HIGHLIGHT";
        case APP_STATE_BUTTON_PRESSED: return "BUTTON_PRESSED";
        case APP_STATE_SHOWING_MESSAGE: return "SHOWING_MESSAGE";
        case APP_STATE_RETURN_TO_BUTTONS: return "RETURN_TO_BUTTONS";
        default: return "UNKNOWN";
    }
}
