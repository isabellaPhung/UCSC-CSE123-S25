#
# SPDX-FileCopyrightText: 2022 atanisoft (github.com/atanisoft)
#
# SPDX-License-Identifier: MIT
#

# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.20)
set(EXTRA_COMPONENT_DIRS
    # UI_ili9488
  "${CMAKE_CURRENT_LIST_DIR}/.."

  #sqlite3
  "${CMAKE_CURRENT_LIST_DIR}/../../sqlite3/components/esp32-idf-sqlite3"
  "${CMAKE_CURRENT_LIST_DIR}/../../database"
  )    
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(UI-test)
