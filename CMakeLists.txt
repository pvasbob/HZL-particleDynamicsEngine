cmake_minimum_required(VERSION 3.25)

project(HZLParticleDynamicsEngine
    VERSION 0.1.0
    LANGUAGES C CXX
)

include(FetchContent) 
include(CTest)

# Build only GLFW itself, skip its documentation, examples, and tests.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    GLFW
    GIT_REPOSITORY http://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

if(BUILD_TESTING)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
        GIT_SHALLOW TRUE
    )
endif()

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glad)
FetchContent_MakeAvailable(glm)
if(BUILD_TESTING)
    FetchContent_MakeAvailable(Catch2)
endif()

include("${glad_SOURCE_DIR}/cmake/GladConfig.cmake")

glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE
    API gl:core=3.3
    EXTENSIONS NONE
)


find_package(OpenGL REQUIRED)

add_executable(HZLParticleDynamicsEngine
    src/main.cpp
    src/app/Application.cpp
    src/platform/GlfwWindow.cpp
    src/renderer/Camera.cpp
    src/renderer/OpenGLMesh.cpp
    src/renderer/OpenGLProgram.cpp
    src/renderer/OpenGLRenderer.cpp
    src/scene/Container.cpp
    src/simulation/ParticleFactory.cpp
    src/simulation/ParticleSystem.cpp
    src/simulation/UniformGrid.cpp
)


set_target_properties(HZLParticleDynamicsEngine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY
        "$<TARGET_FILE_DIR:HZLParticleDynamicsEngine>"
)


target_compile_features(HZLParticleDynamicsEngine PRIVATE cxx_std_20)

target_include_directories(HZLParticleDynamicsEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HZLParticleDynamicsEngine PRIVATE  
    glfw
    glad_gl_core_33
    glm::glm
    OpenGL::GL
)

add_custom_command(TARGET HZLParticleDynamicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:HZLParticleDynamicsEngine>/assets"
)

if(BUILD_TESTING)
    add_executable(HZLParticleDynamicsEngineTests
        tests/ParticleSystemTests.cpp
        src/simulation/ParticleSystem.cpp        
        src/simulation/UniformGrid.cpp
    )

    target_compile_features(
        HZLParticleDynamicsEngineTests
        PRIVATE
            cxx_std_20
    )

    target_include_directories(
        HZLParticleDynamicsEngineTests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(
        HZLParticleDynamicsEngineTests
        PRIVATE
            Catch2::Catch2WithMain
            glm::glm
    )

    add_test(
        NAME ParticleSystemTests
        COMMAND HZLParticleDynamicsEngineTests
    )

endif()

add_executable(HZLParticleDynamicsBenchmark
    benchmarks/ParticleSystemBenchmark.cpp
    src/simulation/ParticleSystem.cpp
    src/simulation/UniformGrid.cpp
)

target_compile_features(
    HZLParticleDynamicsBenchmark
    PRIVATE
        cxx_std_20
)

target_include_directories(
    HZLParticleDynamicsBenchmark
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    HZLParticleDynamicsBenchmark
    PRIVATE
        glm::glm
)
