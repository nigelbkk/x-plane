cmake_minimum_required(VERSION 3.10)
project(Debounce)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DLIN)

# Path to your X-Plane SDK and X-Plane install
set(XPLM_SDK_PATH "/home/nigel/development/XPlaneSDK")
set(XPLANE_PLUGIN_DIR "/media/nigel/XP12/ubuntu/X-Plane-12/Resources/plugins/Debounce/lin_x64")

include_directories(
    ${XPLM_SDK_PATH}/CHeaders
    ${XPLM_SDK_PATH}/CHeaders/XPLM
    ${XPLM_SDK_PATH}/CHeaders/Widgets
    ${XPLM_SDK_PATH}/CHeaders/Utilities
)

file(MAKE_DIRECTORY ${XPLANE_PLUGIN_DIR})

add_library(Debounce SHARED src/debounce.cpp)

# Output directly to the X-Plane plugins folder
set_target_properties(Debounce PROPERTIES
    PREFIX ""                      # Remove "lib" prefix
    SUFFIX ".xpl"                   # Correct extension
    RUNTIME_OUTPUT_DIRECTORY ${XPLANE_PLUGIN_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${XPLANE_PLUGIN_DIR}
)
