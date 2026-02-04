#include "mqtt_handler.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "MQTT_HANDLER";
static esp_mqtt_client_handle_t client;
static mqtt_cmd_callback_t global_cb = NULL;

static void mqtt_event_handler(void *args, esp_event_base_t base, int32_t id, void *data) {
    esp_mqtt_event_handle_t event = data;
    if (id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Connected to Broker");
        esp_mqtt_client_subscribe(client, "controller/xbox", 0);
    } else if (id == MQTT_EVENT_DATA && global_cb) {
        global_cb(event->topic, event->data, event->data_len);
    }
}

void mqtt_init_ext(const char *uri, const char *user, const char *pass, mqtt_cmd_callback_t cb) {
    global_cb = cb;
    const esp_mqtt_client_config_t cfg = {
        .broker.address.uri = uri,
        .credentials.username = user,
        .credentials.authentication.password = pass
    };
    client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_send_telemetry(uint8_t motor_idx, dshot_telemetry_t *data) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", motor_idx);
    cJSON_AddNumberToObject(root, "rpm", data->rpm);
    cJSON_AddNumberToObject(root, "volt", data->voltage_mv / 1000.0);
    cJSON_AddNumberToObject(root, "curr", data->current_ma / 1000.0);
    cJSON_AddNumberToObject(root, "temp", data->temp_c);
    
    char *buf = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client, "sensors/esp32", buf, 0, 0, 0);
    cJSON_free(buf);
    cJSON_Delete(root);
}