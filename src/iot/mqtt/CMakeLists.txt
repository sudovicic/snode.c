cmake_minimum_required(VERSION 3.0)

add_subdirectory(types)
add_subdirectory(packets)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(MQTT_CPP ControlPacket.cpp ControlPacketFactory.cpp SocketContext.cpp
             Topic.cpp
)
set(MQTT_H ControlPacket.h ControlPacketFactory.h SocketContext.h Topic.h)

add_library(mqtt SHARED ${MQTT_CPP} ${MQTT_H})
add_library(snodec::mqtt ALIAS mqtt)

target_link_libraries(mqtt PRIVATE snodec::mqtt-types snodec::mqtt-packets)

target_include_directories(
    mqtt
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
           "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
           "$<BUILD_INTERFACE:${LIBMAGIC_INCLUDE_DIRS}>"
           "$<INSTALL_INTERFACE:include/snode.c>"
)

set_target_properties(mqtt PROPERTIES SOVERSION 1 OUTPUT_NAME snodec-mqtt)

install(
    TARGETS mqtt
    EXPORT snodec_mqtt_Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/snode.c/iot/mqtt
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/snode.c/iot/mqtt
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "cmake" EXCLUDE
)

install(
    EXPORT snodec_mqtt_Targets
    FILE snodec_mqtt_Targets.cmake
    NAMESPACE snodec::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/snodec
)
