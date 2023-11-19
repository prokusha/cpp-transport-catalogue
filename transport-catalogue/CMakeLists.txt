cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
 
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(TRANSPORT_CATALOGUE transport_catalogue.cpp transport_catalogue.h)

set(TRANSPORT_ROUTER transport_router.cpp transport_router.h)

set(UTILIES geo.cpp geo.h ranges.h)

set(SVG svg.cpp svg.h)

set(DOMAIN domain.cpp domain.h)

set(JSON json.cpp json.h)

set(JSON_READER json_reader.cpp json_reader.h)

set(JSON_BUILDER json_builder.cpp json_builder.h)

set(REQUEST request_handler.cpp request_handler.h)

set(MAP_RENDER map_renderer.cpp map_renderer.h)

set(SERIALIZATION serialization.cpp serialization.h)


add_executable(transport_catalogue main.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE} ${TRANSPORT_ROUTER} ${UTILIES} ${SERIALIZATION}
                                            ${SVG} ${DOMAIN} ${JSON} ${JSON_READER} ${JSON_BUILDER} ${REQUEST} ${MAP_RENDER})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
