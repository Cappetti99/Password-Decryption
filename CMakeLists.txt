cmake_minimum_required(VERSION 3.12)
project(Password-Decryption)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trova OpenMP
find_package(OpenMP REQUIRED)

# Flags di ottimizzazione
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Executables (solo i file che esistono realmente)
add_executable(Password8ParallelRandomPassword Password8ParallelRandomPassword.cpp)
add_executable(Password8ParallelRandomPasswordNOWAIT Password8ParallelRandomPasswordNOWAIT.cpp)

add_executable(Password8Sequenziale Password8Sequenziale.cpp)

# Link libraries per tutti gli executables
set(ALL_TARGETS
    Password8ParallelRandomPassword
    Password8ParallelRandomPasswordNOWAIT
    Password8Sequenziale
)

foreach(target ${ALL_TARGETS})
    target_link_libraries(${target} PRIVATE crypt OpenMP::OpenMP_CXX)

    # Abilita supporto OpenMP cancellation
    if(OpenMP_CXX_FOUND)
        target_compile_options(${target} PRIVATE ${OpenMP_CXX_FLAGS})
    endif()
endforeach()

# Messaggio informativo sul build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Optimization flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
