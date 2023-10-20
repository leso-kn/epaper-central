#
#  SPDX-License-Identifier: MIT
#  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
#
#  epaper-central - "epaperd" reference program CMake build file.
#

add_executable(epaperd
    epaperd.c
    util.c)

target_link_libraries(epaperd PRIVATE
    epaper-central)

install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/epaperd
    DESTINATION bin)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/default.ppm
    DESTINATION ${CMAKE_INSTALL_DATADIR}/epaperd)

add_compile_definitions(EPAPERD_DATA_DIR="${CMAKE_INSTALL_DATADIR}/epaperd")
