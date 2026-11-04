cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME wcs_controller)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/valve_controller.c
    Core/Src/linear_actuator_controller.c
    Core/Src/linear_actuator_manual_control.c
    Core/Src/water_sensor.c
    Core/Src/water_sensor_adc.c
    Core/Src/water_sensor_telemetry.c
    third_party/segger_rtt/RTT/SEGGER_RTT.c
    third_party/segger_rtt/RTT/SEGGER_RTT_printf.c
)

# Generate the firmware version header from the Git state at configure time.
set(FW_VERSION "unknown")
set(FW_GIT_COMMIT "unknown")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR} describe --tags --always --dirty
        OUTPUT_VARIABLE FW_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE FW_VERSION_RESULT
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse --short HEAD
        OUTPUT_VARIABLE FW_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE FW_COMMIT_RESULT
    )
    if(NOT FW_VERSION_RESULT EQUAL 0)
        set(FW_VERSION "unknown")
    endif()
    if(NOT FW_COMMIT_RESULT EQUAL 0)
        set(FW_GIT_COMMIT "unknown")
    endif()
endif()
set(FW_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
configure_file(cmake/fw_version.h.in ${CMAKE_BINARY_DIR}/generated/fw_version.h @ONLY)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -Wl,--undefined=fw_version_banner)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/segger_rtt/RTT
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/segger_rtt/Config
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
