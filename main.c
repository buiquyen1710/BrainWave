#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_spp_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#define UART_NUM UART_NUM_1
#define BUF_SIZE 1024
#define TGAM_RXD 16
#define TGAM_TXD 17
#define TGAM_BAUD_RATE 57600

#define SPP_SERVER_NAME "TGAM_BT_SERVER"
static const char *TAG = "TGAM_BT_APP";
static uint32_t spp_client = 0;

static void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = TGAM_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TGAM_TXD, TGAM_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(TAG, "Bluetooth SPP Initialized");
            esp_bt_dev_set_device_name("ESP32_TGAM");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
            break;
        case ESP_SPP_START_EVT:
            ESP_LOGI(TAG, "SPP Server Started");
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            ESP_LOGI(TAG, "Client Connected");
            spp_client = param->srv_open.handle;
            break;
        case ESP_SPP_CLOSE_EVT:
            ESP_LOGI(TAG, "Client Disconnected");
            spp_client = 0;
            break;
        default:
            break;
    }
}

static void tg_read_task(void *arg) {
    uint8_t data[BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, sizeof(data), 100 / portTICK_PERIOD_MS);
        if (len > 0 && spp_client != 0) {
            esp_spp_write(spp_client, len, data);  // gửi dữ liệu TGAM cho app
        }
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    uart_init();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE)); // Sử dụng Classic BT
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_spp_register_callback(spp_callback));
    ESP_ERROR_CHECK(esp_spp_init(ESP_SPP_MODE_CB));

    xTaskCreate(tg_read_task, "tg_read_task", 4096, NULL, 10, NULL);
}
