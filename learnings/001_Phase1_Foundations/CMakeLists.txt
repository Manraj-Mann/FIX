project(Server)

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/server.cpp
)

set(CLIENT_SOURCES
    src/main.cpp
    src/client.cpp
)

set(SERVER_HEADERS
    include/server.h
    include/tcp_connection.h
)

set(CLIENT_HEADERS
    include/client.h
    include/tcp_connection.h
)

# Create executable
add_executable(server ${SERVER_SOURCES} ${SERVER_HEADERS})
add_executable(client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

# Include directories
target_include_directories(server PRIVATE include)
target_include_directories(client PRIVATE include)

# Add compile definitions based on target
target_compile_definitions(server PRIVATE BUILD_SERVER)
target_compile_definitions(client PRIVATE BUILD_CLIENT)