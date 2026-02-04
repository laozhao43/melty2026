#ifndef DSHOT_RMT_H
#define DSHOT_RMT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DSHOT_3D_NEUTRAL     1048
#define DSHOT_CMD_STOP       0

typedef struct {
    uint16_t erpm;
    uint16_t rpm;
    uint16_t voltage_mv;
    uint16_t current_ma;
    int8_t   temp_c;
    bool     valid;
} dshot_telemetry_t;

typedef struct {
    rmt_channel_handle_t tx_chan;
    rmt_channel_handle_t rx_chan;
    rmt_encoder_handle_t encoder;
    dshot_telemetry_t    last_telem;
    _Atomic bool         new_telem;
    uint16_t             magnets;
} dshot_motor_t;

esp_err_t dshot_init_motor(gpio_num_t gpio, dshot_motor_t *motor, uint16_t magnet_count);
void dshot_write(dshot_motor_t *motor, uint16_t throttle, bool telemetry_req);
bool dshot_get_telemetry(dshot_motor_t *motor, dshot_telemetry_t *out_data);
uint16_t dshot_map_3d(float input);

#ifdef __cplusplus
}
#endif
#endif