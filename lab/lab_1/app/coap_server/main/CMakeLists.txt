# Source files and includes
idf_component_register(SRCS "src/coap_handlers.c" "src/coap_server.c" "src/main.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES wifi_station coap nvs_flash pthread)