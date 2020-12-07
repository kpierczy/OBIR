/* ============================================================================================================
 *  File: mqtt_handler.c
 *  Author: Krzysztof Pierczyk
 *  Modified time: 2020-12-08 00:35:56
 *  Description: Implementation of the handlers called from the esp-mqtt library.
 * ============================================================================================================ */

#include "mqtt_client.h"
#include "esp_log.h"

/* -------------------------------------------------- Config -------------------------------------------------- */

// Source file's tag
static char *TAG = "mqtt_handler";


/* ----------------------------------------------- Declarations ----------------------------------------------- */

static void mqtt_event_handler_before_connected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void        mqtt_event_handler_connected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void     mqtt_event_handler_disconnected(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void            mqtt_event_handler_error(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void       mqtt_event_handler_subscribed(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void     mqtt_event_handler_unsubscribed(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void        mqtt_event_handler_published(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void             mqtt_event_handler_data(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);


/* ------------------------------------------------- Functions ------------------------------------------------ */

void mqtt_register_events_handlers(esp_mqtt_client_handle_t client){

    /**
     * @note: @p client structure is passed as the context data at every registration.
     *    It means that the handler will be able to access this structure using the
     *    @p handler_args argument.
     */

    esp_mqtt_client_register_event(client, MQTT_EVENT_BEFORE_CONNECT, mqtt_event_handler_before_connected, client);
    esp_mqtt_client_register_event(client,      MQTT_EVENT_CONNECTED,        mqtt_event_handler_connected, client);
    esp_mqtt_client_register_event(client,   MQTT_EVENT_DISCONNECTED,     mqtt_event_handler_disconnected, client);
    esp_mqtt_client_register_event(client,          MQTT_EVENT_ERROR,            mqtt_event_handler_error, client);
    esp_mqtt_client_register_event(client,     MQTT_EVENT_SUBSCRIBED,       mqtt_event_handler_subscribed, client);
    esp_mqtt_client_register_event(client,   MQTT_EVENT_UNSUBSCRIBED,     mqtt_event_handler_unsubscribed, client);
    esp_mqtt_client_register_event(client,      MQTT_EVENT_PUBLISHED,        mqtt_event_handler_published, client);
    esp_mqtt_client_register_event(client,           MQTT_EVENT_DATA,             mqtt_event_handler_data, client);
}


/* ---------------------------------------------- Static Functions -------------------------------------------- */

/**
 * @brief: Handler called before establishing a connection between client and the broker. 
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_before_connected(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "Before connected event called");
}


/**
 * @brief: Handler called when the connection between client and the broker successes.
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_connected(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
}


/**
 * @brief: Handler called when the client is disconnected from the broker
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_disconnected(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
}

/**
 * @brief: Handler called when protocol error occurs
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_error(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
}

/**
 * @brief: Handler called when the client will sucessfully subscribe to the topic.
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_subscribed(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    esp_mqtt_event_handle_t event = 
        (esp_mqtt_event_handle_t) event_data;
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
}

/**
 * @brief: Handler called when the client will sucessfully unsubscribe the topic.
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_unsubscribed(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    esp_mqtt_event_handle_t event = 
        (esp_mqtt_event_handle_t) event_data;
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
}

/**
 * @brief: Handler called when the client will sucessfully publish to the topic.
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_published(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    esp_mqtt_event_handle_t event = 
        (esp_mqtt_event_handle_t) event_data;
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
}   

/**
 * @brief: Handler called when the client will receive the data from the subscribed topic.
 * 
 * @param handler_args:
 *    user's data, established at the handler's registration, that are passed to the
 *    handler at the library's call; address the data buffer can be passed to the
 *    library using a esp_mqtt_client_register_event() function 
 * @param base:
 *    base id of the event to register the handler for
 * @param event_id:
 *    the id of the event to register the handler for
 * @param event_data:
 *    evenr-related data passed to the handler at the dispatch
 */
static void mqtt_event_handler_data(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
}
