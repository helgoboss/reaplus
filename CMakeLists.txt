cmake_minimum_required(VERSION 3.10)
project(reaplus)

# Main target
set(SOURCES
    reaplus/ModelUtil.cpp
    )
set(MAIN_ONLY_SOURCES
    reaplus/util/log.cpp
    reaplus/Action.cpp
    reaplus/Chunk.cpp
    reaplus/Fx.cpp
    reaplus/FxChain.cpp
    reaplus/FxEnable.cpp
    reaplus/FxParameter.cpp
    reaplus/FxPreset.cpp
    reaplus/Guid.cpp
    reaplus/HelperControlSurface.cpp
    reaplus/IncomingMidiEvent.cpp
    reaplus/MasterPlayrate.cpp
    reaplus/MasterTempo.cpp
    reaplus/MidiInputDevice.cpp
    reaplus/MidiMessage.cpp
    reaplus/MidiOutputDevice.cpp
    reaplus/Pan.cpp
    reaplus/Parameter.cpp
    reaplus/Playrate.cpp
    reaplus/Project.cpp
    reaplus/Reaper.cpp
    reaplus/ReaPlusIntegrationTest.cpp
    reaplus/RecordingInput.cpp
    reaplus/RegisteredAction.cpp
    reaplus/Section.cpp
    reaplus/Tempo.cpp
    reaplus/Track.cpp
    reaplus/TrackArm.cpp
    reaplus/TrackMute.cpp
    reaplus/TrackPan.cpp
    reaplus/TrackSelection.cpp
    reaplus/TrackSend.cpp
    reaplus/TrackSendPan.cpp
    reaplus/TrackSendVolume.cpp
    reaplus/TrackSolo.cpp
    reaplus/TrackVolume.cpp
    reaplus/UndoBlock.cpp
    reaplus/utility.cpp
    reaplus/Volume.cpp
    )
set(MAIN_ONLY_LIBS
    ${Boost_LIBRARIES}
    )
add_library(${PROJECT_NAME} ${SOURCES} ${MAIN_ONLY_SOURCES})
target_link_libraries(${PROJECT_NAME} ${MAIN_ONLY_LIBS})

# Test target
set(TEST_SOURCES
    tests.cpp
    )
add_executable(${PROJECT_NAME}-tests ${SOURCES} ${TEST_SOURCES})