cmake_minimum_required(VERSION 3.10)
project(mailio VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Configure version file
configure_file(
    include/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mailio/version.hpp
)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(examples)

# Installation (optional, based on old one)
install(DIRECTORY include/mailio DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/mailio/version.hpp DESTINATION include/mailio)
