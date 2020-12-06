#include "mqtt_client.h"

/**
 * @brief: Regsters all callbacks for the MQTT-events handling
 * 
 * @param client:
 *    clien't description
 */
void mqtt_register_events_handlers(esp_mqtt_client_handle_t client);