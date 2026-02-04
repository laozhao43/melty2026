#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "cJSON.h"

#include "dshot_rmt.h"
#include "mqtt_handler.h"

static const char *TAG = "ROBOT_PROD";

// --- 核心配置 (已更新为 192.168.1.101) ---
#define WIFI_SSID       "海斯比HSb108"
#define WIFI_PASS       "hsb123456"
#define MQTT_BROKER_URL "mqtt://192.168.1.101:1883" 
#define MQTT_USER       "myuser"
#define MQTT_PASS       "123"
#define TOPIC_CONTROL   "controller/xbox"

// --- 硬件引脚 ---
#define MOTOR_L_GPIO    GPIO_NUM_4
#define MAGNET_COUNT    14
#define CONTROL_HZ      100

// --- 状态位 ---
#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t s_wifi_event_group;

typedef struct { 
    float lx, ly; 
    bool armed; 
} robot_state_t;

static robot_state_t g_bot = { .lx = 0.0f, .ly = 0.0f, .armed = false };
static dshot_motor_t mot_l; 

/**
 * WiFi 事件处理
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
        ESP_LOGI(TAG, "WiFi lost, reconnecting...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * MQTT 指令处理
 */
void xbox_cmd_callback(const char *topic, const char *data, int len) {
    cJSON *json = cJSON_ParseWithLength(data, len);
    if (json) {
        cJSON *lx = cJSON_GetObjectItem(json, "lx");
        cJSON *ly = cJSON_GetObjectItem(json, "ly");
        cJSON *btnA = cJSON_GetObjectItem(json, "btnA");
        cJSON *btnB = cJSON_GetObjectItem(json, "btnB");

        if (lx) g_bot.lx = (float)lx->valuedouble;
        if (ly) g_bot.ly = -(float)ly->valuedouble; 
        if (btnA && btnA->valueint) g_bot.armed = true;
        if (btnB && btnB->valueint) g_bot.armed = false;

        cJSON_Delete(json);
    }
}

/**
 * 核心控制任务
 */
void system_task(void *pv) {
    uint64_t next_loop = esp_timer_get_time();
    const uint32_t interval = 1000000 / CONTROL_HZ;
    dshot_telemetry_t telem;

    while (1) {
        uint16_t throttle = DSHOT_CMD_STOP;
        
        // 只有在 Armed 状态下才计算油门，否则强制为 0
        if (g_bot.armed) {
            throttle = dshot_map_3d(g_bot.ly);
        }

        // 写入电机并请求遥测
        dshot_write(&mot_l, throttle, true);

        // 获取遥测并通过 MQTT 上报
        if (dshot_get_telemetry(&mot_l, &telem)) {
            mqtt_send_telemetry(0, &telem);
        }

        next_loop += interval;
        int64_t sleep = next_loop - esp_timer_get_time();
        if (sleep > 0) vTaskDelay(pdMS_TO_TICKS(sleep / 1000));
        else next_loop = esp_timer_get_time();
    }
}

void app_main(void) {
    // 1. 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. 网络初始化
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // 3. 配置 Wi-Fi 并锁定 BSSID
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .bssid_set = true,
            .bssid = {0x80, 0xAE, 0x54, 0x6B, 0x37, 0x92},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 4. 等待连接成功
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // 5. 初始化 DShot 电机
    memset(&mot_l, 0, sizeof(dshot_motor_t));
    if (dshot_init_motor(MOTOR_L_GPIO, &mot_l, MAGNET_COUNT) != ESP_OK) {
        ESP_LOGE(TAG, "电机初始化失败！");
        return;
    }

    // 6. 初始化 MQTT
    mqtt_init_ext(MQTT_BROKER_URL, MQTT_USER, MQTT_PASS, xbox_cmd_callback);

    // 7. 启动控制任务
    xTaskCreate(system_task, "sys_mgr", 8192, NULL, 15, NULL);
}