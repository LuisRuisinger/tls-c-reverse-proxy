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
        include/tls.h
        src/requesthandler.c
        src/hashmap.c
        include/hashmap.h
        src/handling/readhandler.c
        include/handling/readhandler.h
        src/handling/writehandler.c
        include/handling/writehandler.h
        src/handling/upstreamhandler.c
        include/handling/upstreamhandler.h
        src/handling/requesthandler.c
        include/handling/requesthandler.h
        src/handling/fieldparser.c
        include/handling/fieldparser.h
        include/handling/parserwrapper.h)

# Link against OpenSSL
target_link_libraries(c_reverse_proxy OpenSSL::SSL OpenSSL::Crypto)