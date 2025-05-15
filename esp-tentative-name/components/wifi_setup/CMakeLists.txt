idf_component_register(
  SRCS
    "wifi_setup.c"
  PRIV_REQUIRES
    "esp_event"
    "nvs_flash"
    "esp_wifi"
    "esp_http_server"
    "dns_server"
  INCLUDE_DIRS "."
  EMBED_TXTFILES
    "${CMAKE_CURRENT_LIST_DIR}/root.html"
)
