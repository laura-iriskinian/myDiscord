cmake_minimum_required(VERSION 3.28)
project(myDiscord C)

set(CMAKE_C_STANDARD 11)

# Find the necessary packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LIBPQ REQUIRED libpq)

# Define inclusion directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/client)
include_directories(${CMAKE_SOURCE_DIR}/server)  # Ajout du répertoire server pour les inclusions

# Server executables
add_executable(myDiscord_server
    server/server_main.c
    server/server.c
    server/channel.c 
    server/crypto.c 
    server/database.c 
    server/message.c 
    server/user.c
    server/load_env.c
)

# Configure server
target_include_directories(myDiscord_server PRIVATE ${LIBPQ_INCLUDE_DIRS})
target_link_libraries(myDiscord_server PRIVATE ${LIBPQ_LIBRARIES} pthread)

# Client executables
add_executable(myDiscord_client
    client/main.c 
    client/client.c
    client/interface.c
)

# Configuration of GTK4 for the client
target_include_directories(myDiscord_client PRIVATE 
    ${GTK4_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/client    # pour trouver les en-têtes locaux
    ${CMAKE_SOURCE_DIR}/server    # Ajout du répertoire server pour les inclusions
)
target_link_libraries(myDiscord_client PRIVATE ${GTK4_LIBRARIES})
target_compile_options(myDiscord_client PRIVATE ${GTK4_CFLAGS_OTHER})

# Add pthread for client
target_link_libraries(myDiscord_client PRIVATE pthread)

# CSS style sheet copied in build directory
configure_file(${CMAKE_SOURCE_DIR}/style.css ${CMAKE_BINARY_DIR}/style.css COPYONLY)