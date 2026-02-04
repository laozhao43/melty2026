#include "dshot_rmt.h"
#include <string.h>
#include <stdatomic.h>
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DSHOT_DRV";

// GCR 5B/4B Decoding Table for DShot Telemetry
static const uint8_t gcr_decode[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0x01, 0x04, 0x05, 0xFF, 0xFF, 0x06, 0x07, 
    0xFF, 0xFF, 0x08, 0x09, 0x02, 0x03, 0x0A, 0x0B, 
    0xFF, 0xFF, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0xFF  
};

static uint8_t telem_crc8(uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x07;
            else crc <<= 1;
        }
    }
    return crc;
}

static bool IRAM_ATTR rmt_rx_done_cb(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    dshot_motor_t *motor = (dshot_motor_t *)user_data;
    
    // Debug: Monitor RX symbols in idf.py monitor
    static uint32_t debug_cnt = 0;
    if (++debug_cnt >= 1000) {
        debug_cnt = 0;
        esp_rom_printf("RX ISR: %d symbols\n", (int)edata->num_symbols);
    }

    if (edata->num_symbols == 110) { 
        uint8_t bytes[11] = {0};
        int bit_ptr = 0;
        for (int i = 0; i < 22; i++) {
            uint8_t gcr = 0;
            for (int j = 0; j < 5; j++) {
                // With invert_out=1, high duration means released (Logic 0)
                // We determine bit value by comparing durations
                gcr = (gcr << 1) | (edata->received_symbols[bit_ptr].duration0 > edata->received_symbols[bit_ptr].duration1 ? 1 : 0);
                bit_ptr++;
            }
            uint8_t val = gcr_decode[gcr & 0x1F];
            if (val == 0xFF) return false;
            if (i % 2 == 0) bytes[i / 2] |= (val << 4);
            else bytes[i / 2] |= val;
        }
        if (telem_crc8(bytes, 10) == bytes[10]) {
            motor->last_telem.temp_c = (int8_t)bytes[0];
            motor->last_telem.voltage_mv = (bytes[1] << 8) | bytes[2];
            motor->last_telem.current_ma = (bytes[3] << 8) | bytes[4];
            motor->last_telem.erpm = (bytes[7] << 8) | bytes[8];
            if (motor->magnets > 0) motor->last_telem.rpm = motor->last_telem.erpm / (motor->magnets / 2);
            motor->last_telem.valid = true;
            atomic_store(&motor->new_telem, true);
        }
    }
    return false;
}

esp_err_t dshot_init_motor(gpio_num_t gpio, dshot_motor_t *motor, uint16_t magnet_count) {
    motor->magnets = magnet_count;
    
    // PHASE 1: Sanitize GPIO for ESC startup
    // We set Open-Drain and Level High. This releases the line to your 2k pull-up.
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD, 
        .pin_bit_mask = (1ULL << gpio),
        .pull_down_en = 0,
        .pull_up_en = 0, 
    };
    gpio_config(&io_conf);
    gpio_set_level(gpio, 1); 
    vTaskDelay(pdMS_TO_TICKS(10)); // Brief pause to let ESC see a clean "High"

    // PHASE 2: Initialize RMT TX
    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, 
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
        .flags.invert_out = 1, // Logic 1 pulls line Low, Logic 0 releases line High
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&tx_cfg, &motor->tx_chan), TAG, "TX error");

    // PHASE 3: Initialize RMT RX
    rmt_rx_channel_config_t rx_cfg = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,
        .mem_block_symbols = 128,
    };
    ESP_RETURN_ON_ERROR(rmt_new_rx_channel(&rx_cfg, &motor->rx_chan), TAG, "RX error");

    rmt_rx_event_callbacks_t cbs = { .on_recv_done = rmt_rx_done_cb };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(motor->rx_chan, &cbs, motor));

    // PHASE 4: Encoder for DShot600 (1.67us per bit)
    // level0=1 means Pull-Low, level1=0 means Release-High
    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = { .duration0 = 6, .level0 = 1, .duration1 = 11, .level1 = 0 },
        .bit1 = { .duration0 = 12, .level0 = 1, .duration1 = 5, .level1 = 0 },
        .flags.msb_first = true
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&enc_cfg, &motor->encoder), TAG, "Encoder error");
    
    ESP_ERROR_CHECK(rmt_enable(motor->tx_chan));
    ESP_ERROR_CHECK(rmt_enable(motor->rx_chan));
    
    ESP_LOGI(TAG, "DShot 3D Initialized on GPIO %d (Bidir Mode)", gpio);
    return ESP_OK;
}

void dshot_write(dshot_motor_t *motor, uint16_t throttle, bool telemetry_req) {
    if (!motor || !motor->tx_chan) return;

    uint16_t data = (throttle << 1) | (telemetry_req ? 1 : 0);
    // CRITICAL: Bidirectional DShot requires the 4-bit CRC to be INVERTED
    uint8_t crc = (data ^ (data >> 4) ^ (data >> 8)) & 0x0F;
    uint16_t frame = (data << 4) | ((~crc) & 0x0F);
    
    uint16_t tx_buffer = __builtin_bswap16(frame); 
    
    rmt_transmit_config_t tx_config = { 
        .loop_count = 0,
        .flags.eot_level = 0 // Ensure line is released (High) after transmission
    };
    rmt_transmit(motor->tx_chan, motor->encoder, &tx_buffer, 2, &tx_config);
}

bool dshot_get_telemetry(dshot_motor_t *motor, dshot_telemetry_t *out_data) {
    if (atomic_exchange(&motor->new_telem, false)) {
        memcpy(out_data, &motor->last_telem, sizeof(dshot_telemetry_t));
        return true;
    }
    return false;
}

uint16_t dshot_map_3d(float input) {
    if (input > -0.01f && input < 0.01f) return DSHOT_CMD_STOP;
    if (input >= 0.01f) {
        float t = (input - 0.01f) / 0.99f;
        return 1049 + (uint16_t)(t * 998.0f);
    } else {
        float t = (-input - 0.01f) / 0.99f;
        return 48 + (uint16_t)(t * 999.0f);
    }
}