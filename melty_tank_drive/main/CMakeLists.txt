# Define the source files and header directories for the main component
# Adding 'driver' and 'esp_driver_rmt' (for IDF 5.x) to resolve the missing header errors

idf_component_register(
    SRCS 
        "main.c" 
        "dshot_rmt.c" 
        "mqtt_handler.c"
    INCLUDE_DIRS 
        "."
    PRIV_REQUIRES 
        nvs_flash 
        esp_wifi 
        esp_event 
        esp_netif 
        mqtt 
        esp_timer 
        json
        driver          # Required for gpio.h
        esp_driver_rmt  # Required for RMT in ESP-IDF 5.x
)