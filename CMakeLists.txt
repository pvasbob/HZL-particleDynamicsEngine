cmake_minimum_required(VERSION 3.25)

project(HZLParticleDynamicsEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

include(FetchContent)

# Build only GLFW itself, skip its documentation, examples, and tests.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY http://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(HZLParticleDynamicsEngine
    src/main.cpp
)


target_compile_features(HZLParticleDynamicsEngine PRIVATE cxx_std_20)

target_link_libraries(HZLParticleDynamicsEngine PRIVATE  
    glfw
    OpenGL::GL
)

