/* ============================================================================================================
 *  File: mqtt_utility.h
 *  Author: Krzysztof Pierczyk
 *  Modified time: 2020-12-08 00:29:25
 *  Description: Headers of MQTT-related functions
 * ============================================================================================================ */

#include "mqtt_client.h"

/**
 * @brief: Regsters all callbacks for the MQTT-events handling
 * 
 * @param client:
 *    clien't description
 */
void mqtt_register_events_handlers(esp_mqtt_client_handle_t client);