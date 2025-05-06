idf_component_register(SRCS "*.c"
                    INCLUDE_DIRS ".")
                target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-error=implicit-function-declaration)
