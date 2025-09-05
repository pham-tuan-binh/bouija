#include "led.h"
#include "esp_log.h"
#include "string.h"
#include "ctype.h"

static const char *TAG = "LED";
static led_strip_handle_t led_strip = NULL;
static uint8_t current_brightness = LED_DEFAULT_BRIGHTNESS;

esp_err_t led_init(void)
{
    // LED strip common configuration
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,  // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_COUNT,       // The number of LEDs in the strip
        .led_model = LED_STRIP_MODEL,      // LED strip model, it determines the bit timing
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format is G-R-B
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip RMT specific configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000,     // 10MHz resolution
        .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
    };

    // LED Strip object handle
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    // Clear all LEDs
    led_clear_all();
    
    return ESP_OK;
}

esp_err_t led_deinit(void)
{
    if (led_strip) {
        led_clear_all();
        led_strip_del(led_strip);
        led_strip = NULL;
    }
    return ESP_OK;
}

esp_err_t led_clear_all(void)
{
    if (!led_strip) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    for (int i = 0; i < LED_STRIP_COUNT; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 0, 0, 0));
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    
    return ESP_OK;
}

esp_err_t led_set_color(uint32_t color)
{
    if (!led_strip) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Apply brightness to the color
    uint32_t adjusted_color = led_apply_brightness(color, current_brightness);
    uint8_t r = (adjusted_color >> 16) & 0xFF;
    uint8_t g = (adjusted_color >> 8) & 0xFF;
    uint8_t b = adjusted_color & 0xFF;

    for (int i = 0; i < LED_STRIP_COUNT; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, r, g, b));
    }
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    
    return ESP_OK;
}

esp_err_t led_set_led(int index, uint32_t color)
{
    if (!led_strip) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (index < 0 || index >= LED_STRIP_COUNT) {
        ESP_LOGE(TAG, "LED index %d out of range (0-%d)", index, LED_STRIP_COUNT - 1);
        return ESP_ERR_INVALID_ARG;
    }

    // Apply brightness to the color
    uint32_t adjusted_color = led_apply_brightness(color, current_brightness);
    uint8_t r = (adjusted_color >> 16) & 0xFF;
    uint8_t g = (adjusted_color >> 8) & 0xFF;
    uint8_t b = adjusted_color & 0xFF;

    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, index, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    
    return ESP_OK;
}

int char_to_led_index(char c)
{
    // Convert to uppercase for case-insensitive mapping
    c = toupper(c);
    
    if (c >= 'A' && c <= 'Z') {
        return c - 'A'; // A=0 … Z=25
    }
    if (c >= '0' && c <= '9') {
        return 26 + (c - '0'); // 0=26 … 9=35
    }
    return -1; // invalid character
}

int word_to_led_index(const char *word)
{
    if (!word) return -1;
    
    // Convert to lowercase for case-insensitive comparison
    char lower_word[32];
    strncpy(lower_word, word, sizeof(lower_word) - 1);
    lower_word[sizeof(lower_word) - 1] = '\0';
    
    for (int i = 0; lower_word[i]; i++) {
        lower_word[i] = tolower(lower_word[i]);
    }
    
    if (strcmp(lower_word, "cap") == 0) return 36;
    if (strcmp(lower_word, "wassup") == 0) return 37;
    if (strcmp(lower_word, "i'm out") == 0) return 38;
    if (strcmp(lower_word, "slap") == 0) return 39;
    
    return -1; // invalid word
}

esp_err_t led_show_character(char c)
{
    int led_index = char_to_led_index(c);
    if (led_index == -1) {
        ESP_LOGW(TAG, "Invalid character: %c", c);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Showing character '%c' on LED %d with fade effect", c, led_index);
    
    // Clear all LEDs first
    led_clear_all();
    vTaskDelay(pdMS_TO_TICKS(LED_CLEAR_DELAY_MS));
    
    // Use fade in/out effect for the character
    return led_fade_in_out(led_index, LED_COLOR_WHITE, LED_DISPLAY_DURATION_MS);
}

esp_err_t led_show_word(const char *word)
{
    if (!word) {
        ESP_LOGE(TAG, "Word is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    int led_index = word_to_led_index(word);
    if (led_index == -1) {
        ESP_LOGW(TAG, "Invalid word: %s", word);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Showing word '%s' on LED %d with fade effect", word, led_index);
    
    // Clear all LEDs first
    led_clear_all();
    vTaskDelay(pdMS_TO_TICKS(LED_CLEAR_DELAY_MS));
    
    // Use fade in/out effect for the word
    return led_fade_in_out(led_index, LED_COLOR_CYAN, LED_DISPLAY_DURATION_MS);
}

esp_err_t led_show_text_sequence(const char *text, uint32_t color)
{
    if (!text) {
        ESP_LOGE(TAG, "Text is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Starting text sequence: %s", text);
    
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        
        // Skip whitespace, punctuation, and non-printable characters
        if (isspace(c) || ispunct(c) || c < 32 || c > 126) {
            ESP_LOGI(TAG, "Skipping character: '%c' (ASCII %d)", c, (int)c);
            continue;
        }
        
        int led_index = char_to_led_index(c);
        if (led_index != -1) {
            ESP_LOGI(TAG, "Displaying '%c' on LED %d with fade effect", c, led_index);
            
            // Clear all LEDs
            led_clear_all();
            vTaskDelay(pdMS_TO_TICKS(LED_CLEAR_DELAY_MS));
            
            // Use fade in/out effect for the character
            led_fade_in_out(led_index, color, LED_DISPLAY_DURATION_MS);
        } else {
            ESP_LOGI(TAG, "Skipping unsupported character: '%c' (ASCII %d) - not in A-Z or 0-9 range", c, (int)c);
        }
    }
    
    // Clear all LEDs at the end
    led_clear_all();
    
    return ESP_OK;
}

esp_err_t led_animate_text(const char *text, uint32_t color, int delay_ms)
{
    if (!text) {
        ESP_LOGE(TAG, "Text is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (delay_ms <= 0) {
        delay_ms = LED_DISPLAY_DURATION_MS;
    }

    ESP_LOGI(TAG, "Starting animated text sequence: %s (delay: %dms)", text, delay_ms);
    
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        
        // Skip whitespace and punctuation for LED display
        if (isspace(c) || ispunct(c)) {
            ESP_LOGI(TAG, "Skipping character: %c", c);
            continue;
        }
        
        int led_index = char_to_led_index(c);
        if (led_index != -1) {
            ESP_LOGI(TAG, "Displaying '%c' on LED %d with fade effect", c, led_index);
            
            // Clear all LEDs
            led_clear_all();
            vTaskDelay(pdMS_TO_TICKS(LED_CLEAR_DELAY_MS));
            
            // Use fade in/out effect for the character
            led_fade_in_out(led_index, color, delay_ms);
        } else {
            ESP_LOGW(TAG, "Cannot display character: %c", c);
        }
    }
    
    // Clear all LEDs at the end
    led_clear_all();
    
    return ESP_OK;
}

uint32_t led_color_from_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

esp_err_t led_fade_in_out(int index, uint32_t color, int duration_ms)
{
    if (index < 0 || index >= LED_STRIP_COUNT) {
        ESP_LOGE(TAG, "LED index %d out of range", index);
        return ESP_ERR_INVALID_ARG;
    }

    if (!led_strip) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Apply brightness to the color first
    uint32_t adjusted_color = led_apply_brightness(color, current_brightness);
    uint8_t r = (adjusted_color >> 16) & 0xFF;
    uint8_t g = (adjusted_color >> 8) & 0xFF;
    uint8_t b = adjusted_color & 0xFF;

    // Fade in
    for (int brightness = 0; brightness <= 255; brightness += 5) {
        uint8_t current_r = (r * brightness) / 255;
        uint8_t current_g = (g * brightness) / 255;
        uint8_t current_b = (b * brightness) / 255;
        
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, index, current_r, current_g, current_b));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(duration_ms / 100));
    }

    // Fade out
    for (int brightness = 255; brightness >= 0; brightness -= 5) {
        uint8_t current_r = (r * brightness) / 255;
        uint8_t current_g = (g * brightness) / 255;
        uint8_t current_b = (b * brightness) / 255;
        
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, index, current_r, current_g, current_b));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(duration_ms / 100));
    }

    return ESP_OK;
}

esp_err_t led_set_brightness(uint8_t brightness)
{
    if (brightness > LED_MAX_BRIGHTNESS) {
        brightness = LED_MAX_BRIGHTNESS;
    }
    
    current_brightness = brightness;
    ESP_LOGI(TAG, "LED brightness set to %d", brightness);
    
    return ESP_OK;
}

uint8_t led_get_brightness(void)
{
    return current_brightness;
}

uint32_t led_apply_brightness(uint32_t color, uint8_t brightness)
{
    if (brightness > LED_MAX_BRIGHTNESS) {
        brightness = LED_MAX_BRIGHTNESS;
    }
    
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    // Apply brightness scaling
    r = (r * brightness) / LED_MAX_BRIGHTNESS;
    g = (g * brightness) / LED_MAX_BRIGHTNESS;
    b = (b * brightness) / LED_MAX_BRIGHTNESS;
    
    return (r << 16) | (g << 8) | b;
}


esp_err_t led_loading_sequence(void)
{
    ESP_LOGI(TAG, "Starting LED loading sequence...");
    
    // Clear all LEDs first
    led_clear_all();
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Light up each LED progressively like a loading bar with rainbow colors
    for (int i = 0; i < LED_STRIP_COUNT - 4; i++) {
        // Create rainbow effect by cycling through hue
        uint32_t hue = (i * 360) / LED_STRIP_COUNT; // 0-360 degrees
        uint32_t color = led_hsv_to_rgb(hue, 255, 255); // Full saturation and value
        led_set_led(i, color);
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms delay between each LED
    }
    
    // Hold the rainbow loading bar for a moment
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Turn off all LEDs
    led_clear_all();
    vTaskDelay(pdMS_TO_TICKS(200));
    
    ESP_LOGI(TAG, "Loading sequence completed");
    return ESP_OK;
}

esp_err_t led_pulse_special_leds(void)
{
    ESP_LOGI(TAG, "Starting special LEDs pulse sequence...");
    
    // Clear all LEDs first
    led_clear_all();
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Special LEDs with different colors
    int special_leds[] = {LED_SLAP, LED_CAP, LED_SUP, LED_PEACE};
    uint32_t base_colors[] = {LED_COLOR_RED, LED_COLOR_CYAN, LED_COLOR_YELLOW, LED_COLOR_GREEN};
    
    // Pulse each LED with fade in/out effect
    for (int i = 0; i < 4; i++) {
        ESP_LOGI(TAG, "Pulsing LED %d with color", special_leds[i]);
        led_fade_in_out(special_leds[i], base_colors[i], 1000); // 1 second fade
        vTaskDelay(pdMS_TO_TICKS(200)); // Small delay between LEDs
    }
    
    ESP_LOGI(TAG, "Special LEDs pulse sequence completed");
    return ESP_OK;
}


esp_err_t led_device_init_sequence(void)
{
    ESP_LOGI(TAG, "Starting device initialization sequence...");
    
    // Step 1: Loading sequence
    led_loading_sequence();
    
    // Step 2: Pulse special LEDs (this now handles touch detection internally)
    led_pulse_special_leds();
    
    // All special LEDs are off, clear everything
    led_clear_all();
    
    ESP_LOGI(TAG, "All special LEDs turned off. Proceeding to AI generation...");
    
    return ESP_OK;
}

uint32_t led_hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t r, g, b;
    
    if (s == 0) {
        // Grayscale
        r = g = b = v;
    } else {
        uint8_t region = h / 43;
        uint8_t remainder = (h - (region * 43)) * 6;
        
        uint8_t p = (v * (255 - s)) >> 8;
        uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
        uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
        
        switch (region) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
    }
    
    // Apply brightness to the RGB values
    uint32_t color = (r << 16) | (g << 8) | b;
    return led_apply_brightness(color, current_brightness);
}

