idf_component_register(SRCS "main.c" "touch.c" "led.c" "app_flow.c"
                    INCLUDE_DIRS ""  LDFRAGMENTS "../linker.lf"
                    REQUIRES driver esp_driver_ledc esp_driver_gpio)

# https://github.com/espressif/esp-idf/issues/11696#issuecomment-1596208414
target_compile_options(${COMPONENT_LIB} PRIVATE -fno-if-conversion)