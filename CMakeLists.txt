cmake_minimum_required(VERSION 3.10)
project(Ambient_Light_Project)

find_package(OpenCV REQUIRED)

add_executable(AmbientLight src/main.cpp)

# Meant for httplib.h to function
target_link_libraries(AmbientLight PRIVATE 
    ${OpenCV_LIBS}
    ws2_32
    crypt32
)