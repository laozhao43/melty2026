#include "dshot_handler.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>

static const char *TAG = "DSHOT_RMT";

// RMT Handles
static rmt_channel_handle_t tx_channel = NULL;
static rmt_channel_handle_t rx_channel = NULL;
static rmt_encoder_handle_t dshot_encoder = NULL;

// Shared Telemetry State
static dshot_telemetry_t last_telemetry = {0};
static _Atomic bool telemetry_updated = false;

// CRC-4 Calculation for DShot
static uint8_t dshot_crc4(uint16_t data) {
    return (data ^ (data >> 4) ^ (data >> 8)) & 0x0F;
}

// ISR Callback for RMT RX
static bool IRAM_ATTR dshot_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    // Basic GCR decoding would happen here to extract RPM/Full Telemetry
    // For brevity, we acknowledge the data receipt
    telemetry_updated = true;
    return false;
}

esp_err_t dshot_init(int gpio) {
    // 1. Setup TX Channel
    rmt_tx_channel_config_t tx_config = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000, // 10MHz
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &tx_channel));

    // 2. Setup RX Channel (for Bidirectional)
    rmt_rx_channel_config_t rx_config = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10000000,
        .mem_block_symbols = 128,
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_config, &rx_channel));

    rmt_rx_event_callbacks_t cbs = { .on_recv_done = dshot_rx_done_callback };
    rmt_rx_register_event_callbacks(rx_channel, &cbs, NULL);

    ESP_ERROR_CHECK(rmt_enable(tx_channel));
    ESP_ERROR_CHECK(rmt_enable(rx_channel));

    ESP_LOGI(TAG, "DShot RMT initialized on GPIO %d", gpio);
    return ESP_OK;
}

void dshot_send_throttle(uint16_t throttle, bool telemetry_req) {
    if (throttle > 2047) throttle = 2047;
    
    uint16_t packet = (throttle << 1) | (telemetry_req ? 1 : 0);
    uint8_t crc = dshot_crc4(packet);
    uint16_t frame = (packet << 4) | crc;

    // Use rmt_transmit with a simple bytes encoder or raw symbols
    // Note: In a full implementation, you'd use the rmt_bytes_encoder_config_t
    // to map bits to DSHOT timing (T0H, T0L, T1H, T1L)
    rmt_transmit_config_t transmit_config = { .loop_count = 0 };
    rmt_transmit(tx_channel, dshot_encoder, &frame, sizeof(frame), &transmit_config);
}

bool dshot_get_telemetry(dshot_telemetry_t *out_data) {
    if (telemetry_updated) {
        memcpy(out_data, &last_telemetry, sizeof(dshot_telemetry_t));
        telemetry_updated = false;
        return true;
    }
    return false;
}