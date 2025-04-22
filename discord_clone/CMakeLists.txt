cmake_minimum_required(VERSION 3.10)
project(discord_clone_c)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(POSTGRESQL REQUIRED libpq)
pkg_check_modules(WEBSOCKETS REQUIRED libwebsockets)

include_directories(${GTK4_INCLUDE_DIRS} ${POSTGRESQL_INCLUDE_DIRS} ${WEBSOCKETS_INCLUDE_DIRS})

link_directories(${GTK4_LIBRARY_DIRS} ${POSTGRESQL_LIBRARY_DIRS} ${WEBSOCKETS_LIBRARY_DIRS})

add_executable(discord_clone
    frontend/main.c
    backend/server.c
    backend/database.c
)

target_link_libraries(discord_clone
    ${GTK4_LIBRARIES}
    ${POSTGRESQL_LIBRARIES}
    ${WEBSOCKETS_LIBRARIES}
)
