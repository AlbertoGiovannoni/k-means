cmake_minimum_required(VERSION 3.26)
project(kmeans)

set(CMAKE_CXX_STANDARD 23)

# include OpenMP
find_package(OpenMP REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Aggiungi il percorso della directory SFML dove si trova SFMLConfig.cmake
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:/Users/arian/CLionProjects/librerie/SFML-2.6.1-windows-vc17-64-bit/SFML-2.6.1/lib/cmake/SFML/)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(${SFML_INCLUDE_DIRS})

# File sorgente per la versione sequenziale
add_executable(kmeans_sequential sequential.cpp)

# File sorgente per la versione parallela
add_executable(kmeans_parallel parallel.cpp)

# File sorgente per la versione SoA
add_executable(kmeans_structure_of_array structure_of_array.cpp)

# Include SFML
find_package(SFML 2.5 COMPONENTS system window graphics network audio REQUIRED)
include_directories(${SFML_INCLUDE_DIRS})

# Link SFML alle versioni dell'eseguibile
target_link_libraries(kmeans_sequential sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
target_link_libraries(kmeans_structure_of_array sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
target_link_libraries(kmeans_parallel sfml-system sfml-window sfml-graphics sfml-audio sfml-network)