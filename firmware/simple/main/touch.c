#include "touch.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TOUCH";

// Touch sensor configuration
static touch_pad_t touch_pads[] = {TOUCH_GPIO_SLAP, TOUCH_GPIO_CAP, TOUCH_GPIO_SUP, TOUCH_GPIO_PEACE};
static const char* touch_names[] = {"SLAP", "CAP", "SUP", "PEACE"};

esp_err_t touch_init(void)
{
    ESP_LOGI(TAG, "Initializing touch sensors...");
    
    // Initialize touch pad peripheral
    ESP_ERROR_CHECK(touch_pad_init());
    
    // Set voltage range for touch pads
    ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V));
    
    // Configure touch pad GPIOs
    for (int i = 0; i < 4; i++) {
        ESP_ERROR_CHECK(touch_pad_config(touch_pads[i]));
        ESP_ERROR_CHECK(touch_pad_set_thresh(touch_pads[i], TOUCH_THRESHOLD));
        ESP_LOGI(TAG, "Configured touch pad %s on GPIO %d with threshold %d", touch_names[i], touch_pads[i], TOUCH_THRESHOLD);
    }
    
    // Configure and enable touch pad filter to reduce noise
    touch_filter_config_t filter_info = {
        .mode = TOUCH_PAD_FILTER_IIR_16,
        .debounce_cnt = 1,
        .noise_thr = 0,
        .jitter_step = 4,
        .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2
    };
    ESP_ERROR_CHECK(touch_pad_filter_set_config(&filter_info));
    ESP_ERROR_CHECK(touch_pad_filter_enable());
    
    // Start touch pad FSM
    ESP_ERROR_CHECK(touch_pad_fsm_start());
    
    ESP_LOGI(TAG, "Touch sensors initialized successfully");
    return ESP_OK;
}

esp_err_t touch_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing touch sensors...");
    
    // Stop touch pad FSM
    ESP_ERROR_CHECK(touch_pad_fsm_stop());
    
    // Disable touch pad filter
    ESP_ERROR_CHECK(touch_pad_filter_disable());
    
    // Deinitialize touch pad
    ESP_ERROR_CHECK(touch_pad_deinit());
    
    ESP_LOGI(TAG, "Touch sensors deinitialized successfully");
    return ESP_OK;
}

bool touch_is_pressed(int touch_pin)
{
    uint32_t touch_value;
    esp_err_t ret = touch_pad_filter_read_smooth(touch_pin, &touch_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read touch pad %d: %s", touch_pin, esp_err_to_name(ret));
        return false;
    }
    
    // Debug: Show touch values
    ESP_LOGD(TAG, "Touch pad %d: value=%lu, threshold=%d", touch_pin, touch_value, TOUCH_THRESHOLD);
    
    // Touch is pressed when value is above threshold (normal touch behavior)
    bool pressed = touch_value > TOUCH_THRESHOLD;
    if (pressed) {
        ESP_LOGI(TAG, "Touch pad %d PRESSED! value=%lu > threshold=%d", touch_pin, touch_value, TOUCH_THRESHOLD);
    }
    
    return pressed;
}

button_t touch_get_pressed_button(void)
{
    for (int i = 0; i < 4; i++) {
        if (touch_is_pressed(touch_pads[i])) {
            ESP_LOGI(TAG, "Button %s pressed", touch_names[i]);
            return (button_t)i;
        }
    }
    return BUTTON_NONE;
}

// Debug function to continuously monitor all touch values
void touch_debug_monitor(void)
{
    ESP_LOGI(TAG, "=== Touch Debug Monitor ===");
    ESP_LOGI(TAG, "Threshold: %d", TOUCH_THRESHOLD);
    
    for (int i = 0; i < 4; i++) {
        uint32_t raw_value, filtered_value;
        esp_err_t ret1 = touch_pad_read_raw_data(touch_pads[i], &raw_value);
        esp_err_t ret2 = touch_pad_filter_read_smooth(touch_pads[i], &filtered_value);
        
        if (ret1 == ESP_OK && ret2 == ESP_OK) {
            ESP_LOGI(TAG, "Touch pad %s (GPIO %d): raw=%lu, filtered=%lu, pressed=%s", 
                     touch_names[i], touch_pads[i], raw_value, filtered_value,
                     (filtered_value > TOUCH_THRESHOLD) ? "YES" : "NO");
        } else {
            ESP_LOGE(TAG, "Failed to read touch pad %s (GPIO %d): raw_ret=%s, filter_ret=%s", 
                     touch_names[i], touch_pads[i], 
                     (ret1 != ESP_OK) ? esp_err_to_name(ret1) : "OK",
                     (ret2 != ESP_OK) ? esp_err_to_name(ret2) : "OK");
        }
    }
    ESP_LOGI(TAG, "=========================");
}

const char* touch_get_button_name(button_t button)
{
    if (button >= 0 && button < 4) {
        return touch_names[button];
    }
    return "UNKNOWN";
}
