cmake_minimum_required(VERSION 3.5)

# Additional components
set(EXTRA_COMPONENT_DIRS $ENV{PROJECT_HOME}/lab/lab_1/app/components)

# Obligatory include
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# Project's name
project(coap_server)
