#include <stdio.h>
#include <inttypes.h>
#include "esp_spiffs.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include <time.h>
#include "llm.h"
#include "led.h"
#include <string.h>

static const char *TAG = "MAIN";


/**
 * @brief intializes SPIFFS storage
 * 
 */
void init_storage(void)
{

    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/data",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}


/**
 * @brief Callbacks once generation is done
 * 
 * @param tk_s The number of tokens per second generated
 */
void generate_complete_cb(float tk_s)
{
    ESP_LOGI(TAG, "Generation complete: %.2f tok/s", tk_s);
}

/**
 * @brief Callback for each generated token - displays on LED strip
 * 
 * @param token_str The generated token string
 */
void on_token_generated(const char* token_str)
{
    if (!token_str) return;
    
    ESP_LOGI(TAG, "Token generated: '%s' (len=%d)", token_str, strlen(token_str));
    
    // Debug: print each character's ASCII value
    for (int i = 0; i < strlen(token_str); i++) {
        ESP_LOGI(TAG, "  Char %d: '%c' (ASCII %d)", i, token_str[i], (int)token_str[i]);
    }
    
    // Display the token on LED strip
    // For single characters, show directly
    if (strlen(token_str) == 1) {
        led_show_character(token_str[0]);
    } else {
        // For multi-character tokens, show each character sequentially
        led_show_text_sequence(token_str, LED_COLOR_WHITE);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 LLM application");
    ESP_LOGI(TAG, "Loading Model...");
    init_storage();
    
    // Initialize LED strip
    ESP_LOGI(TAG, "Initializing LED strip...");
    esp_err_t ret = led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED strip: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "LED strip initialized successfully");
        
        // Set LED brightness (0-255, where 255 is maximum brightness)
        // You can adjust this value to control the overall brightness
        led_set_brightness(128);  // 50% brightness - adjust as needed
        ESP_LOGI(TAG, "LED brightness set to %d", led_get_brightness());
    }

    // default parameters
    char *checkpoint_path = "/data/stories260K.bin"; // e.g. out/model.bin
    char *tokenizer_path = "/data/tok512.bin";
    float temperature = 1.0f;        // 0.0 = greedy deterministic. 1.0 = original. don't set higher
    float topp = 0.9f;               // top-p in nucleus sampling. 1.0 = off. 0.9 works well, but slower
    int steps = 500;                  // number of steps to run for
    char *prompt = "Once upon a time"; // prompt string
    unsigned long long rng_seed = 0; // seed rng with time by default

    // parameter validation/overrides
    if (rng_seed <= 0)
        rng_seed = (unsigned int)time(NULL);

    // build the Transformer via the model .bin file
    Transformer transformer;
    ESP_LOGI(TAG, "LLM Path is %s", checkpoint_path);
    build_transformer(&transformer, checkpoint_path);
    if (steps == 0 || steps > transformer.config.seq_len)
        steps = transformer.config.seq_len; // override to ~max length

    // build the Tokenizer via the tokenizer .bin file
    Tokenizer tokenizer;
    build_tokenizer(&tokenizer, tokenizer_path, transformer.config.vocab_size);

    // build the Sampler
    Sampler sampler;
    build_sampler(&sampler, transformer.config.vocab_size, temperature, topp, rng_seed);

    // run!
    ESP_LOGI(TAG, "Starting text generation with prompt: '%s'", prompt);
    generate(&transformer, &tokenizer, &sampler, prompt, steps, &generate_complete_cb, &on_token_generated);
}