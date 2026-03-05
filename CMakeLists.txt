idf_component_register(
    SRCS
        "src/mqtt_base.cpp"
        "src/mqtt_initilizer.cpp"
        "src/mqtt_device.cpp"
        "src/wifi_c.cpp"
        "src/wifi_stat.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_wifi esp_netif nvs_flash esp_event esp_http_client esp_http_server mqtt
)