# Source files and includes
idf_component_register(SRCS "src/udp_server.c" "src/main.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES wifi_station nvs_flash pthread)