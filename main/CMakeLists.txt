idf_component_register(
    SRCS "udp_client.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_wifi
)