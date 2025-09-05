#include "app_flow.h"
#include "led.h"
#include "touch.h"
#include "esp_log.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "APP_FLOW";

// Application state
static app_state_t current_state = APP_STATE_INIT;
static button_t last_pressed_button = BUTTON_NONE;
static bool render_loop_started = false;

// Button messages - 10 messages for each button (Magic 8 Ball style answers)
// Each button represents a type of question, with mixed answer types
static const char* button_messages[4][10] = {
    // BUTTON_SLAP messages - Love/Relationships questions
    {
        "IT IS CERTAIN",           // Positive
        "DONT COUNT ON IT",        // Negative
        "ASK AGAIN LATER",         // Neutral
        "FOLLOW YOUR HEART",       // Advice
        "YES DEFINITELY",          // Positive
        "VERY DOUBTFUL",           // Negative
        "REPLY HAZY TRY AGAIN",    // Neutral
        "TRUST YOUR INSTINCTS",    // Advice
        "SIGNS POINT TO YES",      // Positive
        "MY REPLY IS NO"           // Negative
    },
    // BUTTON_CAP messages - Career/Work questions
    {
        "OUTLOOK GOOD",            // Positive
        "BETTER NOT TELL YOU NOW", // Neutral
        "BELIEVE IN YOURSELF",     // Advice
        "NO",                      // Negative
        "MOST LIKELY",             // Positive
        "CONCENTRATE AND ASK AGAIN", // Neutral
        "KEEP PUSHING FORWARD",    // Advice
        "OUTLOOK NOT SO GOOD",     // Negative
        "ABSOLUTELY",              // Positive
        "CANNOT PREDICT NOW"       // Neutral
    },
    // BUTTON_SUP messages - Life/General questions
    {
        "WASSUP THIS IS BINH",     // Personal
        "YOU MAY RELY ON IT",      // Positive
        "MY SOURCES SAY NO",       // Negative
        "STAY POSITIVE",           // Advice
        "AS I SEE IT YES",         // Positive
        "REPLY HAZY TRY AGAIN",    // Neutral
        "DREAMS COME TRUE",        // Advice
        "VERY DOUBTFUL",           // Negative
        "WITHOUT A DOUBT",         // Positive
        "ASK AGAIN LATER"          // Neutral
    },
    // BUTTON_PEACE messages - Future/Destiny questions
    {
        "PEACE MY GOOD BROTHER",   // Personal
        "SUCCESS IS COMING",       // Advice
        "YES",                     // Positive
        "DONT COUNT ON IT",        // Negative
        "TAKE A LEAP OF FAITH",    // Advice
        "BETTER NOT TELL YOU NOW", // Neutral
        "IT IS DECIDEDLY SO",      // Positive
        "MY REPLY IS NO",          // Negative
        "YOU GOT THIS",            // Advice
        "CANNOT PREDICT NOW"       // Neutral
    }
};

// Function to get random message for a button
static const char* get_random_button_message(button_t button)
{
    if (button < 0 || button >= 4) {
        return "ERROR";
    }
    
    int random_index = rand() % 10; // Random number 0-9
    return button_messages[button][random_index];
}

esp_err_t app_flow_init(void)
{
    ESP_LOGI(TAG, "Initializing application flow...");
    
    // Initialize random seed
    srand(time(NULL));
    
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
                current_state = APP_STATE_BUTTON_SHIMMER;
                break;
                
            case APP_STATE_BUTTON_SHIMMER:
                ESP_LOGI(TAG, "State: BUTTON_SHIMMER");
                
                // Start render loop if not already started
                if (!render_loop_started) {
                    xTaskCreate(led_render_loop, "render_loop", 4096, NULL, 5, NULL);
                    render_loop_started = true;
                    vTaskDelay(pdMS_TO_TICKS(100)); // Give render loop time to start
                }
                
                // Enable ambient effect and button shimmer
                led_set_ambient_effect(true);
                led_set_button_shimmer(true);
                
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
                
                // Disable ambient effect and button shimmer
                led_set_ambient_effect(false);
                led_set_button_shimmer(false);
                
                // Start pulsing the pressed button
                led_set_button_pulse(last_pressed_button, true);
                vTaskDelay(pdMS_TO_TICKS(500));
                
                // Get random message for the pressed button
                const char* selected_message = get_random_button_message(last_pressed_button);
                
                // Show the message as text overlay (duration calculated automatically)
                led_set_text_overlay(selected_message, LED_COLOR_WHITE, 0);
                
                // Calculate wait time based on text length (1000ms per character)
                int text_length = strlen(selected_message);
                int wait_time = text_length * 1000;
                vTaskDelay(pdMS_TO_TICKS(wait_time));
                
                // Check if any button was pressed during text display
                button_t interrupt_button = touch_get_pressed_button();
                if (interrupt_button != BUTTON_NONE) {
                    ESP_LOGI(TAG, "Text sequence was interrupted by button %s", touch_get_button_name(interrupt_button));
                }
                
                current_state = APP_STATE_RETURN_TO_BUTTONS;
                break;
                
            case APP_STATE_RETURN_TO_BUTTONS:
                ESP_LOGI(TAG, "State: RETURN_TO_BUTTONS");
                // Clear any button highlights and pulses, then re-enable shimmer effects
                for (int i = 0; i < 4; i++) {
                    led_set_button_highlight(i, false);
                    led_set_button_pulse(i, false);
                }
                led_set_ambient_effect(true);
                led_set_button_shimmer(true);
                current_state = APP_STATE_BUTTON_SHIMMER;
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
        case APP_STATE_BUTTON_SHIMMER: return "BUTTON_SHIMMER";
        case APP_STATE_BUTTON_PRESSED: return "BUTTON_PRESSED";
        case APP_STATE_SHOWING_MESSAGE: return "SHOWING_MESSAGE";
        case APP_STATE_RETURN_TO_BUTTONS: return "RETURN_TO_BUTTONS";
        default: return "UNKNOWN";
    }
}

