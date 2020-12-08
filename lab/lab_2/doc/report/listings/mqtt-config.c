esp_mqtt_client_config_t mqtt_cfg = {
    // Connection config
    .uri       = "mqtt://192.168.0.94:1883",
    .client_id = "esp8266",
    // Session parameters
    .disable_clean_session = false,
    .keepalive             = 5,
    // Last Will & Testament
    .lwt_topic  = "esp8266/status",
    .lwt_msg    = "disconnected",
    .lwt_qos    = 1,
    .lwt_retain = true,
};

// Initialize the MQTT client using the defined configuration
esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
// Register handlers for library's events
mqtt_register_events_handlers(client);
// Run the client
esp_mqtt_client_start(client);