cmake_minimum_required(VERSION 3.26)
project(c_reverse_proxy C)

set(CMAKE_C_STANDARD 23)

# Find required OpenSSL package
find_package(OpenSSL REQUIRED)

# Include directories for header files
include_directories(include)

add_executable(
        c_reverse_proxy
        src/main.c
        src/setup.c
        src/run.c
        src/tpool.c
        src/tls.c
        src/requesthandler.c
        src/hashmap.c
        src/handling/readhandler.c
        src/handling/writehandler.c
        src/handling/upstreamhandler.c
        src/handling/requesthandler.c
        src/handling/fieldparser.c
)

# enables DEBUG macro if in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

# Link against OpenSSL
target_link_libraries(c_reverse_proxy OpenSSL::SSL OpenSSL::Crypto)