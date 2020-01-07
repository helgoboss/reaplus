# Support CMake version shipping with Ubuntu 18.04 LTS
cmake_minimum_required(VERSION 3.10)

# By default use in-repo vcpkg for package searches
set(CMAKE_TOOLCHAIN_FILE "lib/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Must come after toolchain file stuff
project(reaplus VERSION 0.1.0 LANGUAGES CXX)
include_directories(include)

# Main target
find_package(helgoboss-midi 0.1.0 CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
# TODO Remove dependency to helgoboss-learn
find_package(helgoboss-learn 0.1.0 CONFIG REQUIRED)
find_package(rxcpp CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem exception)
find_package(spdlog CONFIG REQUIRED)
add_library(reaplus STATIC
    src/util/log.cpp
    src/util/ReaperConsoleLogSink.cpp
    src/Action.cpp
    src/Chunk.cpp
    src/Fx.cpp
    src/FxChain.cpp
    src/FxEnable.cpp
    src/FxParameter.cpp
    src/FxPreset.cpp
    src/Guid.cpp
    src/HelperControlSurface.cpp
    src/IncomingMidiEvent.cpp
    src/MasterPlayrate.cpp
    src/MasterTempo.cpp
    src/MidiInputDevice.cpp
    src/MidiOutputDevice.cpp
    src/Pan.cpp
    src/Parameter.cpp
    src/Playrate.cpp
    src/Project.cpp
    src/Reaper.cpp
    src/ReaPlusIntegrationTest.cpp
    src/RecordingInput.cpp
    src/RegisteredAction.cpp
    src/Section.cpp
    src/Track.cpp
    src/TrackArm.cpp
    src/TrackMute.cpp
    src/TrackPan.cpp
    src/TrackSelection.cpp
    src/TrackSend.cpp
    src/TrackSendPan.cpp
    src/TrackSendVolume.cpp
    src/TrackSolo.cpp
    src/TrackVolume.cpp
    src/UndoBlock.cpp
    src/utility.cpp
    src/Volume.cpp
    )
target_link_libraries(reaplus
    PUBLIC
    helgoboss-midi::helgoboss-midi
    helgoboss-learn::helgoboss-learn
    spdlog::spdlog
    unofficial::concurrentqueue::concurrentqueue
    ${Boost_LIBRARIES}
    rxcpp
#    boost-optional
#    boost-utility
#    boost-filesystem
    PRIVATE
#    boost-exception
#    boost-range
#    boost-algorithm
    )
# Use generator syntax for INTERFACE-scoped includes to support usage of installed library (typical "Modern CMake"
# pattern, see https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/)
target_include_directories(reaplus
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${Boost_INCLUDE_DIRS}
    lib/reaper
    )
# Disable those terrible min max macros in windows.h
target_compile_definitions(reaplus PRIVATE NOMINMAX)
# We want strict C++-17 (as PUBLIC because we use C++-17 nested namespaces in public headers)
target_compile_features(reaplus PUBLIC cxx_std_17)
set_target_properties(reaplus PROPERTIES CXX_EXTENSIONS OFF)
add_library(reaplus::reaplus ALIAS reaplus)

# Install
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
write_basic_package_version_file(reaplus-config-version.cmake COMPATIBILITY SameMajorVersion)
install(DIRECTORY include/reaplus DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS reaplus
    EXPORT reaplus-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
install(EXPORT reaplus-targets
    NAMESPACE reaplus::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reaplus
    )
install(FILES
    cmake/reaplus-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/reaplus-config-version.cmake
    DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/reaplus
    )

# Tests
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
  endif ()
endif ()
