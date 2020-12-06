#include <string.h>            // Basic string operations
#include <sys/socket.h>        // Sockets-related constants
#include "esp_log.h"           // Logging
#include "mqtt_client.h"       // MQTT client implementation
#include "mqtt_utility.h"      // Handlers for the MQTT events

/* ---------------------------------- Configuration ---------------------------------- */

// Broker's URI
#define BROKER_URI "mqtt://mqtt.eclipse.org:1883"

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "mqtt_client";

/* ------------------------------------- Declarations --------------------------------- */

// Global variables initialization
extern TaskHandle_t main_handler;

/* ------------------------------------ Thread Code ----------------------------------- */

/**
 * @brief Thread running a MQTT client
 * @param pvParameters
 */
void mqtt_thread(void *pvParameters){

    // Wait for main to block on xTaskNotifyTake()
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Configure broker's URI
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URI,
    };
    
    // Initialize the MQTT client using the defined configuration
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // Register handlers for library's events (assign to all events types with ESP_EVENT_ANY_ID)
    mqtt_register_events_handlers(client);
    // Run the client (@note: this call will create a new FreeRTOS task for MQTT events dispatching and return immediately)
    esp_mqtt_client_start(client);

    // Notify main to wake up
    xTaskNotifyGive(main_handler);

    // Delete task itself
    vTaskDelete(NULL);
}
