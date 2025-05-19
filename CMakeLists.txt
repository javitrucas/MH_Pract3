cmake_minimum_required(VERSION 3.16)

project(mh_p1 LANGUAGES CXX)

# Establecer C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Incluir cabeceras
include_directories(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/common)

# Flags de optimización para máxima velocidad (Release)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -DNDEBUG")

# Archivos fuente comunes
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Crear librería con código común
add_library(mh_lib STATIC ${SRC_FILES})

# Ejecutables
add_executable(main ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(main PRIVATE mh_lib)

add_executable(tables ${CMAKE_SOURCE_DIR}/tables.cpp)
target_link_libraries(tables PRIVATE mh_lib)
