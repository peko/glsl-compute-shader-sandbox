add_executable(n-body-life "src/main.cpp")
target_compile_features(n-body-life PRIVATE cxx_std_20)
set_target_properties(n-body-life PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(n-body-life PRIVATE src)
target_link_libraries(n-body-life PRIVATE gcss)
target_link_libraries(n-body-life PRIVATE glfw)
target_link_libraries(n-body-life PRIVATE imgui)
target_link_libraries(n-body-life PRIVATE imgui_glfw_opengl3)

# set cmake source dir macro
target_compile_definitions(n-body-life PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}" CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
