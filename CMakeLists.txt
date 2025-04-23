cmake_minimum_required(VERSION 3.28)
project(MyDiscord C)

set(CMAKE_C_STANDARD 11)

# Trouver GTK4
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# Trouver PostgreSQL
find_package(PostgreSQL REQUIRED)

# Ajouter les headers
include_directories(
    ${GTK4_INCLUDE_DIRS}
    ${PostgreSQL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

link_directories(
    ${GTK4_LIBRARY_DIRS}
    ${PostgreSQL_LIBRARY_DIRS}
)

configure_file(${CMAKE_SOURCE_DIR}/src/log_style.css ${CMAKE_BINARY_DIR}/log_style.css COPYONLY)

# Ajouter les fichiers sources
add_executable(MyDiscord
    src/main.c
    src/gui.c
    src/db.c
)

# Lier les bibliothèques nécessaires
target_link_libraries(MyDiscord
    ${GTK4_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
    pq
    pthread

)

# Ajouter les définitions GTK4
add_definitions(${GTK4_CFLAGS_OTHER})

