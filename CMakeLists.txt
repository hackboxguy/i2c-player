cmake_minimum_required(VERSION 3.10)
project(i2c-player VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Find header files
file(GLOB_RECURSE HEADERS
    "include/*.hpp"
)

# Create executable
add_executable(i2c-player ${SOURCES} ${HEADERS})

# Add compiler warnings
target_compile_options(i2c-player PRIVATE -Wall -Wextra)

# Install target
install(TARGETS i2c-player DESTINATION bin)
install(DIRECTORY examples/ DESTINATION share/i2c-player/examples)
