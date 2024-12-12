# Especifica la versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(BlockDeviceProjectKenny)

# Establece el estándar de C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Agrega el ejecutable
add_executable(SecondPart Main.cpp SystemManager.cpp)
