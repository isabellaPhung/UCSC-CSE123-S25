file(GLOB_RECURSE SOURCES "src/*.c")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES esp32-idf-sqlite3 json fatfs 
)