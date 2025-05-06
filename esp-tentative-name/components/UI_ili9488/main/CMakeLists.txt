idf_component_register(SRCS "main.c" "display_init.c" "ui.c" "helper_menus.c"
                    INCLUDE_DIRS "." "../../database/include" "../../sqlite3/components/esp32-idf-sqlite3/include")
                target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-error=implicit-function-declaration)
