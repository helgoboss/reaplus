cmake_minimum_required(VERSION 3.10)
project(reaplus)
include_directories(include)

# Main target
set(SOURCES
    src/main/ModelUtil.cpp
    )
set(MAIN_ONLY_SOURCES
    src/main/util/log.cpp
    src/main/Action.cpp
    src/main/Chunk.cpp
    src/main/Fx.cpp
    src/main/FxChain.cpp
    src/main/FxEnable.cpp
    src/main/FxParameter.cpp
    src/main/FxPreset.cpp
    src/main/Guid.cpp
    src/main/HelperControlSurface.cpp
    src/main/IncomingMidiEvent.cpp
    src/main/MasterPlayrate.cpp
    src/main/MasterTempo.cpp
    src/main/MidiInputDevice.cpp
    src/main/MidiMessage.cpp
    src/main/MidiOutputDevice.cpp
    src/main/Pan.cpp
    src/main/Parameter.cpp
    src/main/Playrate.cpp
    src/main/Project.cpp
    src/main/Reaper.cpp
    src/main/ReaPlusIntegrationTest.cpp
    src/main/RecordingInput.cpp
    src/main/RegisteredAction.cpp
    src/main/Section.cpp
    src/main/Tempo.cpp
    src/main/Track.cpp
    src/main/TrackArm.cpp
    src/main/TrackMute.cpp
    src/main/TrackPan.cpp
    src/main/TrackSelection.cpp
    src/main/TrackSend.cpp
    src/main/TrackSendPan.cpp
    src/main/TrackSendVolume.cpp
    src/main/TrackSolo.cpp
    src/main/TrackVolume.cpp
    src/main/UndoBlock.cpp
    src/main/utility.cpp
    src/main/Volume.cpp
    )
set(MAIN_ONLY_LIBS
    ${Boost_LIBRARIES}
    )
add_library(${PROJECT_NAME} ${SOURCES} ${MAIN_ONLY_SOURCES})
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} ${MAIN_ONLY_LIBS})

# Test target
set(TEST_SOURCES
    src/test/tests.cpp
    )
add_executable(${PROJECT_NAME}-tests ${SOURCES} ${TEST_SOURCES})