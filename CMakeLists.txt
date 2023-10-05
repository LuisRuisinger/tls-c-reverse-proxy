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
        src/routing.c
        include/routing.h)

# Link against OpenSSL
target_link_libraries(c_reverse_proxy OpenSSL::SSL OpenSSL::Crypto)