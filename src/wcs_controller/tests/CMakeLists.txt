cmake_minimum_required(VERSION 3.22)
project(wcs_valve_host_tests C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

add_executable(test_valve_controller
    test_valve_controller.c
    ../Core/Src/valve_controller.c
)
target_include_directories(test_valve_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/fakes
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc
)
target_compile_options(test_valve_controller PRIVATE -Wall -Wextra -Werror)

add_test(NAME valve_controller COMMAND test_valve_controller)
