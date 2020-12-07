# Source files and includes
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(
    SRCS "src/mqtt_handlers.c" "src/mqtt_client.c" "src/main.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES wifi_station nvs_flash mqtt pthread
    EMBED_TXTFILES ${project_dir}/data/cert/ca.crt
)