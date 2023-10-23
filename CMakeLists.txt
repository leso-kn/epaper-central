#
#  SPDX-License-Identifier: MIT
#  Copyright © 2023 Lesosoftware https://github.com/leso-kn.
#
#  epaper-central - Main CMake build file.
#

cmake_minimum_required(VERSION 3.0)
project(epaper-central VERSION 0.1.0)

# Build Options
option(EPAPER_CENTRAL_BUILD_EPAPERD "Build daemon reference program" ON)

# Sources
set(epaper_central_SOURCES
    src/epaper-central.c
    src/md5.c)

# libepaper-central
add_library(epaper-central_o OBJECT ${epaper_central_SOURCES})

add_library(epaper-central SHARED)
add_library(epaper-central_static STATIC)

target_link_libraries(epaper-central epaper-central_o)
target_link_libraries(epaper-central_static epaper-central_o)

set_target_properties(epaper-central PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

set_target_properties(epaper-central_static PROPERTIES
    OUTPUT_NAME epaper-central)

target_include_directories(epaper-central_o PUBLIC include)

set_property(TARGET epaper-central_o PROPERTY POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(ECTR_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(ECTR_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(ECTR_VERSION_PATCH=${PROJECT_VERSION_PATCH})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/epaper-central.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/epaper-central.pc @ONLY)

if(NOT CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

if(NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif()

if(NOT CMAKE_INSTALL_DATADIR)
    set(CMAKE_INSTALL_DATADIR share)
endif()

if (EPAPER_CENTRAL_BUILD_EPAPERD)
    add_subdirectory(epaperd)
endif()

install(
    TARGETS epaper-central
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    TARGETS epaper-central_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/epaper-central.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

find_package(Doxygen)
if (DOXYGEN_FOUND)

add_custom_target(doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    VERBATIM)

install(
    DIRECTORY docs/man
    DESTINATION ${CMAKE_INSTALL_DATADIR})

endif (DOXYGEN_FOUND)
