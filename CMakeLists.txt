cmake_minimum_required(VERSION 3.12)
project(openmp-hello-world)

set(CMAKE_CXX_STANDARD 11)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")


set(SOURCE_FILES)

add_executable(provapass8 provapass8.cpp)
add_executable(provapass8sequenziale provapass8sequenziale.cpp)
add_executable(provapass8datesequenziale provapass8datesSequenziale.cpp)
add_executable(Password8ParallelProvaOPT Password8ParallelProvaOPT.cpp)
add_executable(provapass8date provapass8dates.cpp)

target_link_libraries(provapass8 PRIVATE crypt)
target_link_libraries(provapass8date PRIVATE crypt)
target_link_libraries(provapass8sequenziale PRIVATE crypt)
target_link_libraries(provapass8datesequenziale PRIVATE crypt)
target_link_libraries(Password8ParallelProvaOPT PRIVATE crypt)



