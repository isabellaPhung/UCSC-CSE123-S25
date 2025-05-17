set(embed_files "${CMAKE_CURRENT_LIST_DIR}/root.html")

if(CONFIG_HTTPS_SERVER)
  list(APPEND embed_files
    "${CMAKE_CURRENT_LIST_DIR}/certs/prvtkey.pem"
    "${CMAKE_CURRENT_LIST_DIR}/certs/servercert.pem"
  )
endif()

idf_component_register(
  SRCS "wifi_setup.c"
  PRIV_REQUIRES
    "esp_event"
    "nvs_flash"
    "esp_wifi"
    "esp_https_server"
    "esp-tls"
    "esp_http_server"
    "dns_server"
  INCLUDE_DIRS "."
  EMBED_TXTFILES "${embed_files}"
)
