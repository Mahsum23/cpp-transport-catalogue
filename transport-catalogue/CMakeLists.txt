cmake_minimum_required(VERSION 3.11)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/transport_catalogue.proto)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

set(SOURCE_FILES src/domain.cpp src/geo.cpp src/json_builder.cpp src/json_reader.cpp src/json.cpp src/map_renderer.cpp src/request_handler.cpp src/serialization.cpp src/svg.cpp src/transport_catalogue.cpp)
set(HEADER_FILES include/domain.h include/geo.h include/json_builder.h include/graph.h include/json_reader.h include/json.h include/map_renderer.h include/ranges.h include/request_handler.h include/router.h include/serialization.h include/svg.h include/transport_catalogue.h)
add_executable(transport-catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${SOURCE_FILES} src/main.cpp)

target_include_directories(transport-catalogue PUBLIC ${Protobuf_INCLUDE_DIRS} ${INCLUDE_DIR})
target_include_directories(transport-catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR} include)

target_link_libraries(transport-catalogue ${Protobuf_LIBRARY} Threads::Threads)

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
target_link_libraries(transport-catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)