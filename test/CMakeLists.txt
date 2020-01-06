find_package(Catch2 CONFIG REQUIRED)
include(Catch)
add_executable(reaplus-tests
    tests.cpp
    )
target_compile_features(reaplus-tests PRIVATE cxx_std_17)
set_target_properties(reaplus-tests PROPERTIES CXX_EXTENSIONS OFF)
# Disable those terrible min max macros in windows.h
target_compile_definitions(reaplus-tests PRIVATE NOMINMAX)
target_link_libraries(reaplus-tests PRIVATE Catch2::Catch2 reaplus::reaplus)
catch_discover_tests(reaplus-tests)