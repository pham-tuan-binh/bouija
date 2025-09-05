#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "led.h"
#include "app_flow.h"
#include <string.h>

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 LED Board Application");
    
    // Initialize LED strip
    ESP_LOGI(TAG, "Initializing LED strip...");
    esp_err_t ret = led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED strip: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "LED strip initialized successfully");
    
    // Set LED brightness
    led_set_brightness(50);  // 50% brightness
    ESP_LOGI(TAG, "LED brightness set to %d", led_get_brightness());
    
    // Initialize application flow (includes touch sensors)
    ESP_LOGI(TAG, "Initializing application flow...");
    ret = app_flow_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize application flow: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "Application flow initialized successfully");
    
    // Start the main application loop
    ESP_LOGI(TAG, "Starting main application loop...");
    app_flow_run();
}