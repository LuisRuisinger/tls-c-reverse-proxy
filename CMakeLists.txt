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
        src/hashmap.c
        src/handling/readhandler.c
        src/handling/writehandler.c
        src/handling/backend/upstreamhandler.c
        src/handling/requesthandler.c
        src/handling/parser/fieldparser.c
        src/handling/parser/uriparser.c
        include/uriparser.h
        src/handling/branching/functions.c
        include/functions.h
        src/handling/pool/pendingpool.c
        include/pendingpool.h
        src/handling/backend/downstreamhandler.c
        include/downstreamhandler.h
        src/handling/responsehandler.c
        include/responsehandler.h
)

# enables DEBUG macro if in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

# Link against OpenSSL
target_link_libraries(c_reverse_proxy OpenSSL::SSL OpenSSL::Crypto)