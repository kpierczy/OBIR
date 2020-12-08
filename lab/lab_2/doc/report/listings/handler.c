static void mqtt_event_handler_data(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");

    esp_mqtt_event_handle_t event = 
        (esp_mqtt_event_handle_t) event_data;

    printf("Received from topic:%.*s. Data is:\n%.*s\r\n", 
        event->topic_len, event->topic,
        event->data_len, event->data
    );
}