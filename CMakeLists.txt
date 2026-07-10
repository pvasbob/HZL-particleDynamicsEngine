cmake_minimum_required(VERSION 3.25)

project(HZLParticleDynamicsEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

add_executable(HZLParticleDynamicsEngine
    src/main.cpp
)


target_compile_features(HZLParticleDynamicsEngine PRIVATE cxx_std_20)

