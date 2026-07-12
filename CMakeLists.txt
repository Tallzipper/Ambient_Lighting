cmake_minimum_required(VERSION 3.10)
project(Ambient_Light_Project)

set(OpenCV_DIR "C:/Users/carli/Downloads/opencv/build/x64/vc16/lib")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(AmbientLight src/main.cpp)

target_link_libraries(AmbientLight ${OpenCV_LIBS})