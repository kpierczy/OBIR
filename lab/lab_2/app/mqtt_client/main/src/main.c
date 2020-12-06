#include "freertos/FreeRTOS.h" // FreeRTOS tasks
#include "esp_log.h"           // Logging mechanisms
#include "nvs_flash.h"         // Non-Volatile storage flash
#include "station.h"           // WiFi connection [auth]

/* ---------------------------------- Configuration ---------------------------------- */

// WiFi access point data
#define WIFI_SSID      "Andrzej"
#define WIFI_PASS      "Gnp64wpewynh"

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "main";

// Global variables initialization
TaskHandle_t main_handler;

/* ------------------------------------- Declarations --------------------------------- */

void mqtt_thread(void *p);

/* ---------------------------------------- Code -------------------------------------- */

/**
 * @brief Main task.
 */
void app_main(){

    // Initialize NVS flash for other components' use
    ESP_ERROR_CHECK(nvs_flash_init());

    // LOG start of the Programm
    ESP_LOGI(TAG, "Connecting to WiFi AP...");

    // Connect via WiFi to the AP
    wifi_connect(WIFI_SSID, WIFI_PASS);

    /**
     * @note: MQTT-Client library uses an esp_event's default loop to interact
     *    with user's handlers. This loop should be created by esp_event_loop_create_default()
     *    call before using any library's API. However, this call is performed inside
     *    the wifi_connect() function (as the WiFi stack also uses this event loop),
     *    thus it is omitted here.
     */

    // Create MQTT client task
    xTaskCreate(mqtt_thread, "mqtt", 1024 * 5, NULL, 5, NULL);
    // Wait for MQTT client task to finish
    main_handler = xTaskGetCurrentTaskHandle();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    /**
     * @note: The esp_mqtt_client_start() call in the MQTT-task will create a new
     *    thread for MQTT-related events dispatching. It means, that the main 
     *    thread will be instantly waken up and finished. As the MQTT-Client
     *    thread keeps the system running, the wifi_disconnect() cannot be called
     *    as it was in case of CoAP server's implementation.
     */
    // // Disconnect from the AP
    // wifi_disconnect();
}