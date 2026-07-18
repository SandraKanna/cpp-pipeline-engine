# 3.25: available in Ubuntu 24.04 LTS and GitHub Actions runners,
# recent enough for presets v6 and modern target-based features.

cmake_minimum_required(VERSION 3.25)
project(cpe VERSION 0.1.0 LANGUAGES CXX)

add_executable(cpe_skeleton src/main.cpp)
target_compile_features(cpe_skeleton PRIVATE cxx_std_20)