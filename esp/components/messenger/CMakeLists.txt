idf_component_register(
    SRCS "messenger.c"
    INCLUDE_DIRS "."
    REQUIRES database json mqtt_conn
)