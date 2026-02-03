# CMakeList.txt : CMake project for Server

# Add source to this project's executable.
file(GLOB_RECURSE SERVER_LINUX_SOURCES
    "src/*.cpp"
    "src/*.c"
    "src/*.cc"
    "src/*.cxx"
)

add_executable(ServerLinux
    ${SERVER_SOURCES}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ServerLinux PROPERTY CXX_STANDARD 17)
endif()

target_link_libraries(ServerLinux
    PRIVATE
        Core
        dep
)


