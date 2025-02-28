cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(BikeTitans)

# Include components directory explicitly (optional step)
set(COMPONENTS_DIR ${CMAKE_SOURCE_DIR}/components)
list(APPEND EXTRA_COMPONENT_DIRS ${COMPONENTS_DIR})