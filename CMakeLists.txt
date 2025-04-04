cmake_minimum_required(VERSION 3.30)
project(Visualizer)

set(CMAKE_CXX_STANDARD 20)

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(CMAKE_PREFIX_PATH  "C:/vcpkg/installed/x64-windows")

#Find required packages
find_package(portaudio CONFIG REQUIRED)
find_package(fftw3 CONFIG REQUIRED)
find_package(Freetype CONFIG REQUIRED)  # Add FreeType
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)

add_executable(Visualizer main.cpp)

#Link libraries
target_link_libraries(Visualizer
PRIVATE
    portaudio
    FFTW3::fftw3
    Freetype::Freetype
    SFML::Network
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::System
)

add_custom_command(TARGET Visualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "C:/vcpkg/installed/x64-windows/debug/bin"
        $<TARGET_FILE_DIR:Visualizer>
)