cmake_minimum_required(VERSION 3.16)

project(KrakenSDR CXX)

set(CMAKE_CXX_STANDARD 14)

# --- Main application
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
