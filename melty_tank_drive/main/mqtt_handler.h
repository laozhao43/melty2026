#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <stdint.h>
#include "dshot_rmt.h"

typedef void (*mqtt_cmd_callback_t)(const char *topic, const char *payload, int len);

/**
 * @brief Initialize MQTT with authentication
 */
void mqtt_init_ext(const char *uri, const char *user, const char *pass, mqtt_cmd_callback_t cb);

/**
 * @brief Send motor telemetry data to MQTT broker
 */
void mqtt_send_telemetry(uint8_t motor_idx, dshot_telemetry_t *data);

#endif