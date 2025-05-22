# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/components/sqlite3/components/esp32-idf-sqlite3"
  "${CMAKE_CURRENT_LIST_DIR}/components/database"
  "${CMAKE_CURRENT_LIST_DIR}/components/UI_ili9488"

  # required for mqtt_conn
  "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/backoffAlgorithm"
  "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/coreMQTT"
  "${CMAKE_CURRENT_LIST_DIR}/components/esp-aws-iot/libraries/common/posix_compat"

  "${CMAKE_CURRENT_LIST_DIR}/components/wifi_setup/components/dns_server"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp-tentative-name)
