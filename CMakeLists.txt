cmake_minimum_required(VERSION 3.20)
project(atmos LANGUAGES C CXX)

option(BUILD_TESTS "build tests" OFF)

# OpenGL
find_package(OpenGL REQUIRED)

# externals
add_subdirectory("externals")

# compute-shader-sandbox
add_library(gcss INTERFACE)
target_include_directories(gcss INTERFACE "include")
target_compile_features(gcss INTERFACE cxx_std_20)
set_target_properties(gcss PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(gcss INTERFACE OpenGL::GL)
target_link_libraries(gcss INTERFACE glad)
target_link_libraries(gcss INTERFACE glfw)
target_link_libraries(gcss INTERFACE imgui)
target_link_libraries(gcss INTERFACE imgui_glfw_opengl3)
target_link_libraries(gcss INTERFACE glm)
target_link_libraries(gcss INTERFACE stb)
target_link_libraries(gcss INTERFACE spdlog::spdlog)

# compile options
target_compile_options(gcss INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)

# sandbox
add_subdirectory("sandbox")

# tests
if(BUILD_TESTS)
  add_subdirectory("tests")
endif()