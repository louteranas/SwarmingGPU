cmake_minimum_required (VERSION 2.8.11)

set(EXEC "boids")

set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
add_executable(${EXEC} main.cxx agent.cxx vector.cxx workspace.cxx)


target_link_libraries(${EXEC} PUBLIC ${OpenCL_LIBRARY})

