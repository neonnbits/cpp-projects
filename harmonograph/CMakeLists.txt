cmake_minimum_required(VERSION 3.10)
project(Harmonograph VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML components
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link SFML libraries (with SFML:: prefix)
target_link_libraries(${PROJECT_NAME} PRIVATE 
                     SFML::Graphics)  # This automatically includes Window and System