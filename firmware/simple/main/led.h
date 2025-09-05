#ifndef LED_H
#define LED_H

#include "esp_err.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// LED strip configuration
#define LED_STRIP_GPIO 8
#define LED_STRIP_COUNT 40
#define LED_STRIP_MODEL LED_MODEL_WS2812


// Special LED indices for slap/cap/sup/peace (last 4 LEDs)
#define LED_PEACE 36
#define LED_SUP 37
#define LED_CAP 38
#define LED_SLAP 39

// Color definitions
#define LED_COLOR_OFF 0x000000
#define LED_COLOR_RED 0xFF0000
#define LED_COLOR_GREEN 0x00FF00
#define LED_COLOR_BLUE 0x0000FF
#define LED_COLOR_WHITE 0xFFFFFF
#define LED_COLOR_YELLOW 0xFFFF00
#define LED_COLOR_CYAN 0x00FFFF
#define LED_COLOR_MAGENTA 0xFF00FF

// Animation timing
#define LED_DISPLAY_DURATION_MS 1000
#define LED_CLEAR_DELAY_MS 100

// Brightness settings
#define LED_MAX_BRIGHTNESS 128  // Maximum brightness (0-255)
#define LED_DEFAULT_BRIGHTNESS 64  // Default brightness (0-255)

// Function declarations
esp_err_t led_init(void);
esp_err_t led_deinit(void);
esp_err_t led_clear_all(void);
esp_err_t led_set_color(uint32_t color);
esp_err_t led_set_led(int index, uint32_t color);
esp_err_t led_show_character(char c);
esp_err_t led_show_word(const char *word);
esp_err_t led_show_text_sequence(const char *text, uint32_t color);
esp_err_t led_animate_text(const char *text, uint32_t color, int delay_ms);

// Character to LED index mapping
int char_to_led_index(char c);
int word_to_led_index(const char *word);

// Utility functions
uint32_t led_color_from_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t led_hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v);
esp_err_t led_fade_in_out(int index, uint32_t color, int duration_ms);

// Brightness control functions
esp_err_t led_set_brightness(uint8_t brightness);
uint8_t led_get_brightness(void);
uint32_t led_apply_brightness(uint32_t color, uint8_t brightness);

// Loading sequence functions
esp_err_t led_loading_sequence(void);
esp_err_t led_pulse_special_leds(void);
esp_err_t led_device_init_sequence(void);

// LED animation functions
esp_err_t led_show_loading_sequence(void);
esp_err_t led_highlight_buttons(void);

// Render system functions
void led_render_loop(void *pvParameters);
esp_err_t led_render_frame(void);
esp_err_t led_set_ambient_effect(bool enabled);
esp_err_t led_set_button_shimmer(bool enabled);
esp_err_t led_set_text_overlay(const char *text, uint32_t color, int duration_ms);
esp_err_t led_set_button_highlight(int button_index, bool highlighted);
esp_err_t led_set_button_pulse(int button_index, bool pulsing);

#endif // LED_H
