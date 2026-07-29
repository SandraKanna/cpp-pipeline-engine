# 3.25: available in Ubuntu 24.04 LTS and GitHub Actions runners,
# recent enough for presets v6 and modern target-based features.

cmake_minimum_required(VERSION 3.25)
project(cpe VERSION 0.1.0 LANGUAGES CXX)

# env vars for external tools like VsCode to know how to compile with CMake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# create the library cpe_core with specific specs to respect: C++20
# Anything that needs Record/Value links against this
add_library(cpe_core src/data_model.cpp)
target_compile_features(cpe_core PUBLIC cxx_std_20)
target_include_directories(cpe_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# create example executable cpe_skeleton, one consumer of cpe_core
# also with specific specs to respect: C++20
add_executable(cpe_skeleton src/main.cpp)
target_compile_features(cpe_skeleton PRIVATE cxx_std_20)

# link the app and the library
target_link_libraries(cpe_skeleton PRIVATE cpe_core)