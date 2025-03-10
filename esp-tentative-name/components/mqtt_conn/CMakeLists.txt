idf_component_register(
  SRCS
    "mqtt_conn.c"
  REQUIRES
    "freertos"
    "backoffAlgorithm"
    "coreMQTT"
    "posix_compat"
  INCLUDE_DIRS "."
  EMBED_TXTFILES
    "${CMAKE_CURRENT_LIST_DIR}/certs/root_cert_auth.crt"
    "${CMAKE_CURRENT_LIST_DIR}/certs/client.crt"
    "${CMAKE_CURRENT_LIST_DIR}/certs/client.key"
)
