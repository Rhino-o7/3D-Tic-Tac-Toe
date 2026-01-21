file(GLOB_RECURSE CORE_SOURCES
    src/*.cpp
)

add_library(Core STATIC ${CORE_SOURCES})

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(Core PUBLIC cxx_std_17)
