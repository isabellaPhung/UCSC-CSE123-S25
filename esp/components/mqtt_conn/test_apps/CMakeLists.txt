# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS
  # required for mqtt_conn
  "${CMAKE_CURRENT_LIST_DIR}/../../esp-aws-iot/libraries/backoffAlgorithm"
  "${CMAKE_CURRENT_LIST_DIR}/../../esp-aws-iot/libraries/coreMQTT"
  "${CMAKE_CURRENT_LIST_DIR}/../../esp-aws-iot/libraries/common/posix_compat"

  # mqtt_conn
  "${CMAKE_CURRENT_LIST_DIR}/.."
  "${CMAKE_CURRENT_LIST_DIR}/../../wifi_setup"
  "${CMAKE_CURRENT_LIST_DIR}/../../wifi_setup/components"
)

project(mqtt-conn-test)
