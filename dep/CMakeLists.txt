# websocketpp and ASIO are header-only libraries when using standalone mode
add_library(dep INTERFACE)

target_include_directories(dep
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/websocketpp-0.8.2
        ${CMAKE_CURRENT_SOURCE_DIR}/asio-1.12.1/include
)

target_compile_features(dep INTERFACE cxx_std_17)

target_compile_definitions(dep
    INTERFACE
        ASIO_STANDALONE
        _WEBSOCKETPP_CPP11_STL_
        _WIN32_WINNT=0x0601
)