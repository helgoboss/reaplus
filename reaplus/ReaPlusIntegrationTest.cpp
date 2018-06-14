#include "ReaPlusIntegrationTest.h"
#include "Reaper.h"
#include "Project.h"
#include "Track.h"
#include "TrackSend.h"
#include "Section.h"
#include "Action.h"
#include "FxChain.h"
#include "Fx.h"
#include "FxParameter.h"
#include "MidiInputDevice.h"
#include "MidiOutputDevice.h"
#include <cstring>
#include <reaper_plugin_functions.h>

using boost::none;
using boost::optional;
using std::string;
using rxcpp::composite_subscription;
using rxcpp::observable;
using std::function;

namespace reaplus {
  void ReaPlusIntegrationTest::tests() {
    // TODO to be tested
    /**
     *    - MidiMessage
          - All events
          - Reaper::sampleCounter
     */


    testWithUntil("Create empty project in new tab", [](auto testIsOver) {
      // Given
      const auto currentProjectBefore = Reaper::instance().currentProject();
      const int projectCountBefore = Reaper::instance().projectCount();

      // When
      optional<Project> eventProject;
      int count = 0;
      Reaper::instance().projectSwitched().take_until(testIsOver).subscribe([&](Project p) {
        count++;
        eventProject = p;
      });
      const auto newProject = Reaper::instance().createEmptyProjectInNewTab();

      // Then
      assertTrue(currentProjectBefore == currentProjectBefore, "Project comparison broken"); // NOLINT
      assertTrue(Reaper::instance().projectCount() == projectCountBefore + 1, "Project count not increased");
      assertTrue(Reaper::instance().projects().as_blocking().count() == projectCountBefore + 1);
      assertTrue(Reaper::instance().currentProject() != currentProjectBefore, "Current project still the same");
      assertTrue(Reaper::instance().currentProject() == newProject, "Current project not new project");
      assertTrue(Reaper::instance().projects().as_blocking().first() != newProject);
      assertTrue(Reaper::instance().projectsWithCurrentOneFirst().as_blocking().first() == newProject);
      assertTrue(Reaper::instance().projectsWithCurrentOneFirst().as_blocking().count() == projectCountBefore + 1);
      assertTrue(newProject.trackCount() == 0, "Project not empty at beginning");
      assertTrue(newProject.index() > 0);
      assertTrue(!newProject.filePath().is_initialized());
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventProject == newProject, "Project event wrong");
    });

    testWithUntil("Add track", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackAdded().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      const auto newTrack = project.addTrack();

      // Then
      assertTrue(project.trackCount() == 1, "Track count not increased");
      assertTrue(newTrack.index() == 0, "Track index wrong");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == newTrack, "Track event wrong");
    });

    test("Query master track", [] {
      // Given
      auto project = Reaper::instance().currentProject();

      // When
      const auto masterTrack = project.masterTrack();

      // Then
      assertTrue(masterTrack.isMasterTrack(), "Track is not master track");
    });

    test("Query all tracks", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      project.addTrack();

      // When
      auto tracks = project.tracks();

      // Then
      assertTrue(tracks.as_blocking().count() == 2, "Not enough tracks returned");
    });

    test("Query track by GUID", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.firstTrack();
      auto newTrack = project.addTrack();

      // When
      auto foundTrack = project.trackByGuid(newTrack.guid());

      // Then
      assertTrue(foundTrack.isAvailable(), "Reported existing track as non-available");
      assertTrue(foundTrack == newTrack, "Didn't find right track");
      assertTrue(foundTrack != firstTrack, "Found wrong track");
      assertTrue(newTrack.guid() == Track::getMediaTrackGuid(newTrack.mediaTrack()),
          "getMediaTrackGuid() doesn't work");
    });

    test("Query non-existent track by GUID", [] {
      // Given
      auto project = Reaper::instance().currentProject();

      // When
      auto foundTrack = project.trackByGuid("noingwrg9898z3h");

      // Then
      assertTrue(!foundTrack.isAvailable(), "Reported non-existent track as available");
    });

    test("Query track project", [] {
      // Given
      auto project = Reaper::instance().currentProject();

      // When
      auto trackProject = project.firstTrack()->project();

      // Then
      assertTrue(trackProject == project, "Track project is not original project");
    });

    test("Query track name", [] {
      // Given
      auto track = firstTrack();

      // When
      auto trackName = track.name();

      // Then
      assertTrue(trackName.empty(), "Wrong name reported");
    });

    testWithUntil("Set track name", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackNameChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.setName("Foo Bla");

      // Then
      assertTrue(track.name() == "Foo Bla", "Name not correctly set");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Query track input monitoring", [] {
      // Given
      auto track = firstTrack();

      // When
      auto mode = track.inputMonitoringMode();

      // Then
      assertTrue(mode == InputMonitoringMode::Normal, "Wrong input monitoring mode");
    });

    testWithUntil("Set track input monitoring", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackInputMonitoringChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.setInputMonitoringMode(InputMonitoringMode::NotWhenPlaying);

      // Then
      assertTrue(track.inputMonitoringMode() == InputMonitoringMode::NotWhenPlaying, "Wrong input monitoring mode");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Query track recording input", [] {
      // Given
      auto track = firstTrack();

      // When
      auto recInput = track.recordingInput();

      // Then
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == none, "Returned explicit channel");
      assertTrue(midiRecInput.device() == none, "Returned explicit device");
      assertTrue(*RecordingInput::ofRecInputIndex(recInput->recInputIndex()) == *recInput, "== doesn't work");
    });

    testWithUntil("Set track recording input 1", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackInputChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.setRecordingInput(MidiRecordingInput::fromDeviceAndChannel(4, 5));

      // Then
      auto recInput = track.recordingInput();
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == 5, "Returned wrong channel");
      assertTrue(midiRecInput.device()->id() == 4, "Returned wrong device");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Set track recording input 2", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setRecordingInput(MidiRecordingInput::fromAllChannelsOfDevice(7));

      // Then
      auto recInput = track.recordingInput();
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == none, "Returned explicit channel");
      assertTrue(midiRecInput.device()->id() == 7, "Returned wrong device");
    });

    test("Set track recording input 3", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setRecordingInput(MidiRecordingInput::fromAllDevicesAndChannels());

      // Then
      auto recInput = track.recordingInput();
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == none, "Returned explicit channel");
      assertTrue(midiRecInput.device() == none, "Returned explicit device");
    });

    test("Set track recording input 4", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setRecordingInput(MidiRecordingInput::fromAllDevicesWithChannel(15));

      // Then
      auto recInput = track.recordingInput();
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == 15, "Returned wrong channel");
      assertTrue(midiRecInput.device() == none, "Returned explicit device");
    });

    test("Query track volume", [] {
      // Given
      auto track = firstTrack();

      // When
      auto volume = track.volume();

      // Then
      assertTrue(volume.reaperValue() == 1.0, "Wrong REAPER value returned");
      assertTrue(volume.db() == 0.0, "Wrong db returned");
      assertTrue(volume.normalizedValue() == 0.71599999999999997, "Wrong normalized value returned");
    });

    testWithUntil("Set track volume", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackVolumeChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.setVolume(0.25);

      // Then
      auto volume = track.volume();
      assertTrue(volume.reaperValue() == 0.031588093366685013, "Wrong REAPER value returned");
      assertTrue(volume.db() == -30.009531739774296, "Wrong db returned");
      assertTrue(volume.normalizedValue() == 0.25000000000003497, "Wrong normalized value returned");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Query track pan", [] {
      // Given
      auto track = firstTrack();

      // When
      auto pan = track.pan();

      // Then
      assertTrue(pan.reaperValue() == 0.0, "Wrong REAPER value returned");
      assertTrue(pan.normalizedValue() == 0.5, "Wrong normalized value returned");
    });

    testWithUntil("Set track pan", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackPanChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.setPan(0.25);

      // Then
      auto pan = track.pan();
      assertTrue(pan.reaperValue() == -0.5, "Wrong REAPER value returned");
      assertTrue(pan.normalizedValue() == 0.25, "Wrong normalized value returned");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Query track selection state", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto track = firstTrack();

      // When
      bool isSelected = track.isSelected();

      // Then
      assertTrue(!isSelected, "Wrong value returned");
      assertTrue(project.selectedTrackCount() == 0);
    });

    testWithUntil("Select track", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto track = firstTrack();
      auto track2 = *project.trackByIndex(2);

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackSelectedChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.select();
      track2.select();

      // Then
      assertTrue(track.isSelected(), "Track was not selected");
      assertTrue(project.selectedTrackCount() == 2);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.firstSelectedTrack()->index() == 0);
      assertTrue(project.selectedTracks().as_blocking().count() == 2);
      assertTrue(count == 2, "Event count wrong");
      assertTrue(*eventTrack == track2, "Track event wrong");
    });

    testWithUntil("Unselect track", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackSelectedChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.unselect();

      // Then
      assertTrue(!track.isSelected(), "Track was not unselected");
      assertTrue(project.selectedTrackCount() == 1);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.firstSelectedTrack()->index() == 2);
      assertTrue(project.selectedTracks().as_blocking().count() == 1);
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Query track auto arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      bool isInAutoArmMode = track.hasAutoArmEnabled();

      // Then
      assertTrue(!isInAutoArmMode, "Wrong value returned");
    });

    test("Query track arm state", [] {
      // Given
      auto track = firstTrack();

      // When
      bool isArmed = track.isArmed();
      bool isArmedIgnoringAutoArm = track.isArmed(false);

      // Then
      assertTrue(!isArmed, "Wrong value returned");
      assertTrue(!isArmedIgnoringAutoArm, "Wrong value returned (ignoring auto-arm)");
    });

    testWithUntil("Arm track in normal mode", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.arm();

      // Then
      assertTrue(track.isArmed(), "Track was not armed");
      assertTrue(track.isArmed(false), "Track was not armed (ignoring auto-arm)");
      assertTrue(!track.hasAutoArmEnabled(), "Track is not in normal mode anymore");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Disarm track in normal mode", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.disarm();

      // Then
      assertTrue(!track.isArmed(), "Track was not disarmed");
      assertTrue(!track.isArmed(false), "Track was not disarmed (ignoring auto-arm)");
      assertTrue(!track.hasAutoArmEnabled(), "Track is not in normal mode anymore");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Enable track auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.enableAutoArm();

      // Then
      assertTrue(track.hasAutoArmEnabled(), "Track is still in normal mode");
      assertTrue(!track.isArmed(), "Track is suddenly armed");
      assertTrue(!track.isArmed(false), "Track is suddenly armed (ignoring auto-arm)");
    });

    testWithUntil("Arm track in auto-arm mode", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.arm();

      // Then
      assertTrue(track.isArmed(), "Track was not armed");
      // TODO Interesting! GetMediaTrackInfo_Value read with I_RECARM seems to support auto-arm already!
      // So maybe we should remove the chunk check and the parameter supportAutoArm
      assertTrue(track.isArmed(false), "Track was not armed (ignoring auto-arm)");
      assertTrue(track.hasAutoArmEnabled(), "Track is not in auto-arm mode anymore");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Disarm track in auto-arm mode", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.disarm();

      // Then
      assertTrue(!track.isArmed(), "Track was not disarmed");
      assertTrue(!track.isArmed(false), "Track was not disarmed (ignoring auto-arm)");
      assertTrue(track.hasAutoArmEnabled(), "Track is not in auto-arm mode anymore");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Disable track auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.disableAutoArm();

      // Then
      assertTrue(!track.hasAutoArmEnabled(), "Track is still in auto-arm mode");
      assertTrue(!track.isArmed(), "Track is suddenly armed");
      assertTrue(!track.isArmed(false), "Track is suddenly armed (ignoring auto-arm)");
    });

    test("Switch to normal track mode while armed", [] {
      // Given
      auto track = firstTrack();
      track.arm();
      assertTrue(track.isArmed(), "Precondition failed");

      // When
      track.disableAutoArm();

      // Then
      assertTrue(!track.hasAutoArmEnabled(), "Track is still in auto-arm mode");
      assertTrue(track.isArmed(), "Track is suddenly unarmed");
      assertTrue(track.isArmed(false), "Track is suddenly unarmed (ignoring auto-arm)");
    });

    test("Switch track to auto-arm mode while armed", [] {
      // Given
      auto track = firstTrack();
      track.unselect();

      // When
      track.enableAutoArm();

      // Then
      assertTrue(track.hasAutoArmEnabled(), "Track is still in normal mode");
      assertTrue(track.isArmed(), "Track is suddenly unarmed");
      assertTrue(track.isArmed(false), "Track is suddenly unarmed (ignoring auto-arm)");
    });

    testWithUntil("Disarm track in auto-arm mode (ignoring auto-arm)", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.disarm(false);

      // Then
      assertTrue(!track.isArmed(), "Track was not disarmed");
      assertTrue(!track.isArmed(false), "Track was not disarmed (ignoring auto-arm)");
      assertTrue(!track.hasAutoArmEnabled(), "Track is still in auto-arm mode");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Arm track in auto-arm mode (ignoring auto-arm)", [](auto testIsOver) {
      // Given
      auto track = firstTrack();
      track.enableAutoArm();
      assertTrue(track.hasAutoArmEnabled(), "Precondition failed");
      assertTrue(!track.isArmed(), "Precondition failed");

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackArmChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      track.arm(false);

      // Then
      assertTrue(track.isArmed(), "Track was not armed");
      assertTrue(track.isArmed(false), "Track was not armed (ignoring auto-arm)");
      assertTrue(!track.hasAutoArmEnabled(), "Track is still in auto-arm mode");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Select track exclusively", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);
      auto thirdTrack = *project.trackByIndex(2);
      firstTrack.unselect();
      secondTrack.select();
      thirdTrack.select();

      // When
      int count = 0;
      Reaper::instance().trackSelectedChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
      });
      firstTrack.selectExclusively();

      // Then
      assertTrue(firstTrack.isSelected(), "First track has not been selected");
      assertTrue(!secondTrack.isSelected(), "Second track is still selected");
      assertTrue(!thirdTrack.isSelected(), "Third track is still selected");
      assertTrue(project.selectedTrackCount() == 1);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.selectedTracks().as_blocking().count() == 1);
      assertTrue(count == 3, "Event count wrong");
    });

    testWithUntil("Remove track", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      int trackCountBefore = project.trackCount();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByNumber(2);
      auto secondTrackGuid = secondTrack.guid();
      assertTrue(firstTrack.isAvailable(), "Precondition failed");
      assertTrue(secondTrack.index() == 1, "Precondition failed");
      assertTrue(secondTrack.isAvailable(), "Precondition failed");

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackRemoved().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      project.removeTrack(firstTrack);

      // Then
      assertTrue(project.trackCount() == trackCountBefore - 1, "Track count still same as before");
      assertTrue(!firstTrack.isAvailable(), "Removed track still available");
      assertTrue(secondTrack.index() == 0, "Index of track after removed track not invalidated");
      assertTrue(secondTrack.guid() == secondTrackGuid, "GUID of track after removed track has changed");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == firstTrack, "Track event wrong");
    });

    test("Query track automation mode", [] {
      // Given
      auto track = firstTrack();

      // When
      auto automationMode = track.automationMode();
      auto globalAutomationOverride = Reaper::instance().globalAutomationOverride();
      auto effectiveAutomationMode = track.effectiveAutomationMode();

      // Then
      assertTrue(automationMode == AutomationMode::TrimRead, "Wrong track automation mode");
      if (globalAutomationOverride == AutomationMode::NoOverride) {
        assertTrue(effectiveAutomationMode == AutomationMode::TrimRead, "Wrong effective track automation mode");
      } else {
        assertTrue(effectiveAutomationMode == globalAutomationOverride, "Wrong effective track automation mode");
      }
    });

    test("Query track send count", [] {
      // Given
      auto track = firstTrack();

      // When
      auto sendCount = track.sendCount();

      // Then
      assertTrue(sendCount == 0, "Wrong send count");
      assertTrue(track.sendByIndex(0) == none, "Send by index returned something");
      assertTrue(!track.sendByTargetTrack(track).isAvailable(), "Send by target returned something");
      assertTrue(!track.indexBasedSendByIndex(0).isAvailable(), "Index-based send by index returned something");
      assertTrue(track.sends().as_blocking().count() == 0, "Sends not empty");
    });

    test("Add track send", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);

      // When
      auto send = firstTrack.addSendTo(secondTrack);

      // Then
      assertTrue(firstTrack.sendCount() == 1, "No send has been added");
      assertTrue(firstTrack.sendByIndex(0).is_initialized(), "Send by index returned nothing");
      assertTrue(*firstTrack.sendByIndex(0) == send, "Send by index returned wrong send");
      assertTrue(firstTrack.sendByTargetTrack(secondTrack).isAvailable(), "Send by target returned noting");
      assertTrue(!secondTrack.sendByTargetTrack(firstTrack).isAvailable(), "Reverse send by target returned something");
      assertTrue(firstTrack.indexBasedSendByIndex(0).isAvailable(), "Index-based send by index returned nothing");
      assertTrue(firstTrack.sends().as_blocking().count() == 1, "Sends returns wrong number");
    });

    test("Query track send", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);
      auto thirdTrack = project.addTrack();

      // When
      auto sendToSecondTrack = firstTrack.sendByTargetTrack(secondTrack);
      auto sendToThirdTrack = firstTrack.addSendTo(thirdTrack);

      // Then
      assertTrue(sendToSecondTrack.isAvailable());
      assertTrue(sendToThirdTrack.isAvailable());
      assertTrue(sendToSecondTrack.index() == 0);
      assertTrue(sendToThirdTrack.index() == 1);
      // assertTrue(sendToSecondTrack.name());
      assertTrue(sendToSecondTrack.sourceTrack() == firstTrack);
      assertTrue(sendToThirdTrack.sourceTrack() == firstTrack);
      assertTrue(sendToSecondTrack.targetTrack() == secondTrack);
      assertTrue(sendToThirdTrack.targetTrack() == thirdTrack);
      assertTrue(sendToSecondTrack.volume().db() == 0);
      assertTrue(sendToThirdTrack.volume().db() == 0);
    });

    testWithUntil("Set track send volume", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto thirdTrack = *project.trackByIndex(2);
      auto send = firstTrack.sendByTargetTrack(thirdTrack);

      // When
      optional<TrackSend> eventTrackSend;
      int count = 0;
      Reaper::instance().trackSendVolumeChanged().take_until(testIsOver).subscribe([&](TrackSend s) {
        count++;
        eventTrackSend = s;
      });
      send.setVolume(0.25);

      // Then
      assertTrue(send.volume().db() == -30.009531739774296);
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrackSend == send, "Track send event wrong");
    });

    testWithUntil("Set track send pan", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto thirdTrack = *project.trackByIndex(2);
      auto send = firstTrack.sendByTargetTrack(thirdTrack);

      // When
      optional<TrackSend> eventTrackSend;
      int count = 0;
      Reaper::instance().trackSendPanChanged().take_until(testIsOver).subscribe([&](TrackSend s) {
        count++;
        eventTrackSend = s;
      });
      send.setPan(0.25);

      // Then
      assertTrue(send.pan().reaperValue() == -0.5, "Wrong REAPER value returned");
      assertTrue(send.pan().normalizedValue() == 0.25, "Wrong normalized value returned");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrackSend == send, "Track send event wrong");
    });

    test("Query action", [] {
      // Given
      firstTrack().selectExclusively();
      assertTrue(!firstTrack().isMuted());

      // When
      auto toggleAction = Reaper::instance().mainSection().actionByCommandId(6);
      auto normalAction = Reaper::instance().mainSection().actionByCommandId(41075);
      auto normalActionByIndex = Reaper::instance().mainSection().actionByIndex(normalAction.index());

      // Then
      assertTrue(toggleAction.isAvailable());
      assertTrue(normalAction.isAvailable());
      assertTrue(toggleAction.character() == ActionCharacter::Toggle);
      assertTrue(normalAction.character() == ActionCharacter::Trigger);
      assertTrue(!toggleAction.isOn());
      assertTrue(!normalAction.isOn());
      assertTrue(toggleAction.parameterType() == ParameterType::Action);
      assertTrue(*toggleAction.clone() == toggleAction);
      assertTrue(toggleAction.commandId() == 6);
      assertTrue(toggleAction.commandName() == "6");
      assertTrue(toggleAction.name() == "Track: Toggle mute for selected tracks");
      assertTrue(toggleAction.index() >= 0);
      assertTrue(toggleAction.section() == Reaper::instance().mainSection());
      assertTrue(normalActionByIndex == normalAction, "== doesn't work");
    });

    testWithUntil("Invoke action", [](auto testIsOver) {
      // Given
      auto action = Reaper::instance().mainSection().actionByCommandId(6);

      // When
      optional<Action> eventAction;
      int count = 0;
      Reaper::instance().actionInvoked().take_until(testIsOver).subscribe([&](Action a) {
        count++;
        eventAction = a;
      });
      action.invoke();

      // Then
      assertTrue(action.isOn());
      assertTrue(firstTrack().isMuted());
      // TODO Actually it would be nice if the actionInvoked event would be raised but it just isn't
      assertTrue(count == 0, "actionInvoked was actually raised!");
      assertTrue(!eventAction.is_initialized(), "actionInvoked was actually raised!");
    });

    testWithUntil("Test actionInvoked event", [](auto testIsOver) {
      // Given
      auto action = Reaper::instance().mainSection().actionByCommandId(1582);

      // When
      optional<Action> eventAction;
      int count = 0;
      Reaper::instance().actionInvoked().take_until(testIsOver).subscribe([&](Action a) {
        count++;
        eventAction = a;
      });
      reaper::Main_OnCommandEx(action.commandId(), 0, nullptr);

      // Then
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventAction == action, "Action event wrong");
    });

    testWithUntil("Unmute track", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackMuteChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      firstTrack().unmute();

      // Then
      assertTrue(!track.isMuted());
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Mute track", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackMuteChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      firstTrack().mute();

      // Then
      assertTrue(track.isMuted());
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Solo track", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackSoloChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      firstTrack().solo();

      // Then
      assertTrue(track.isSolo());
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    testWithUntil("Unsolo track", [](auto testIsOver) {
      // Given
      auto track = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackSoloChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      firstTrack().unsolo();

      // Then
      assertTrue(!track.isSolo());
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == track, "Track event wrong");
    });

    test("Generate GUID", [] {
      // Given

      // When
      auto guid = Reaper::instance().generateGuid();

      // Then
      assertTrue(guid.toString().length() == 36);
    });

    test("Main section functions", [] {
      // Given
      auto section = Reaper::instance().mainSection();

      // When

      // Then
      assertTrue(section.actions().as_blocking().count() == section.actionCount());
    });

    test("Register and unregister action", [] {
      // Given

      // When
      // TODO Rename RegisteredAction to ActionRegistration or something like that
      auto reg = Reaper::instance().registerAction("reaPlusTest", "ReaPlus test action", [] {
      });
      auto action = Reaper::instance().actionByCommandName("reaPlusTest");

      // Then
      assertTrue(action.isAvailable());
      action.invoke();
      assertTrue(action.character() == ActionCharacter::Trigger);
      assertTrue(action.commandId() > 0);
      assertTrue(action.commandName() == "reaPlusTest");
      assertTrue(action.index() >= 0);
      assertTrue(!action.isOn());
      assertTrue(action.name() == "ReaPlus test action");
      reg.unregister();
      assertTrue(!action.isAvailable(), "Action should not be available anymore after unregistering");
    });

    test("Register and unregister toggle action", [] {
      // Given

      // When
      bool isOn = false;
      auto reg = Reaper::instance().registerAction("reaPlusTest2", "ReaPlus test toggle action",
          [&isOn] {
            isOn = !isOn;
          },
          [&isOn] {
            return isOn;
          }
      );
      auto action = Reaper::instance().actionByCommandName("reaPlusTest2");

      // Then
      assertTrue(action.isAvailable());
      assertTrue(!action.isOn());
      action.invoke();
      assertTrue(action.isOn());
      assertTrue(action.character() == ActionCharacter::Toggle);
      assertTrue(action.commandId() > 0);
      assertTrue(action.commandName() == "reaPlusTest2");
      assertTrue(action.index() >= 0);
      assertTrue(action.name() == "ReaPlus test toggle action");
      reg.unregister();
      assertTrue(!action.isAvailable(), "Action should not be available anymore after unregistering");
    });

    auto fxTests = [this](function<Track(void)> getTrack, function<FxChain(void)> getFxChain) {
      test("Query fx chain", [getFxChain] {
        // Given
        auto fxChain = getFxChain();

        // When

        // Then
        assertTrue(fxChain.fxCount() == 0);
        assertTrue(fxChain.fxs().as_blocking().count() == 0);
        assertTrue(fxChain.fxByIndex(0) == none);
        assertTrue(fxChain.firstFx() == none);
        assertTrue(fxChain.lastFx() == none);
        assertTrue(!fxChain.fxByGuid("bla").isAvailable());
        assertTrue(!fxChain.fxByGuidAndIndex("bla", 0).isAvailable());
        assertTrue(fxChain.firstFxByName("bla") == none);
        assertTrue(fxChain.chunk() == none);
      });

      testWithUntil("Add track fx by original name", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxAdded().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        auto fx = fxChain.addFxByOriginalName("ReaControlMIDI (Cockos)");

        // Then
        assertTrue(fx.is_initialized());
        assertTrue(fxChain.fxCount() == 1);
        assertTrue(fxChain.fxs().as_blocking().count() == 1);
        assertTrue(fxChain.fxByIndex(0).is_initialized());
        assertTrue(*fxChain.fxByIndex(0) == *fx);
        assertTrue(fxChain.firstFx().is_initialized());
        assertTrue(*fxChain.firstFx() == *fx);
        assertTrue(fxChain.lastFx().is_initialized());
        assertTrue(*fxChain.lastFx() == *fx);
        assertTrue(!fx->guid().empty());
        assertTrue(fx->guid().length() == 36, "GUID length is wrong");
        assertTrue(fx->guid().find_first_of("{}") == string::npos);
        assertTrue(fxChain.fxByGuid(fx->guid()).isAvailable());
        assertTrue(fxChain.fxByGuid(fx->guid()) == *fx);
        assertTrue(fxChain.fxByGuidAndIndex(fx->guid(), 0).isAvailable());
        assertTrue(fxChain.fxByGuidAndIndex(fx->guid(), 1).isAvailable(),
            "Index has not automatically corrected itself");
        assertTrue(!fxChain.fxByGuidAndIndex("bla", 0).isAvailable());
        assertTrue(fxChain.firstFxByName("ReaControlMIDI (Cockos)").is_initialized());
        assertTrue(*fxChain.firstFxByName("ReaControlMIDI (Cockos)") == *fx);
        auto chainChunk = fxChain.chunk();
        assertTrue(chainChunk.is_initialized());
        assertTrue(chainChunk->startsWith("<FXCHAIN"));
        assertTrue(chainChunk->endsWith("\n>"));
        auto firstTag = chainChunk->findFirstTag(0);
        assertTrue(firstTag.is_initialized());
        assertTrue(firstTag->content() == chainChunk->content());
        assertTrue(count == 1, "Event count wrong");
        assertTrue(*eventFx == fx, "FX event wrong");
      });

      test("Check track fx with 1 fx", [getFxChain, getTrack] {
        // Given
        auto fxChain = getFxChain();
        auto track = getTrack();

        // When
        auto fx1 = fxChain.fxByIndex(0);

        // Then
        assertTrue(fx1.is_initialized());
        assertTrue(fx1->isAvailable());
        assertTrue(fx1->index() == 0);
        assertTrue(fx1->queryIndex() == (fxChain.isInputFx() ? 0x1000000 : 0));
        assertTrue(!fx1->guid().empty());
        assertTrue(fx1->name() == "VST: ReaControlMIDI (Cockos)");
        assertTrue(fx1->chunk().startsWith("BYPASS 0 0 0"));
        assertTrue(fx1->chunk().endsWith("\nWAK 0"));
        assertTrue(fx1->tagChunk().startsWith("<VST \"VST: ReaControlMIDI (Cockos)\" reacontrolmidi"));
        assertTrue(fx1->tagChunk().endsWith("\n>"));
        assertTrue(!fx1->stateChunk().contains("<"));
        assertTrue(!fx1->stateChunk().contains(">"));
        assertTrue(fx1->fileNameWithoutExtension() == "reacontrolmidi");
        assertTrue(fx1->track() == track);
        assertTrue(fx1->isInputFx() == fxChain.isInputFx());
        assertTrue(fx1->chain() == fxChain);
        assertTrue(fx1->parameterCount() == 17);
        assertTrue(fx1->parameters().as_blocking().count() == 17);
        assertTrue(fx1->parameterByIndex(15).isAvailable());
        assertTrue(!fx1->parameterByIndex(17).isAvailable());
        assertTrue(fx1->isEnabled());
      });

      testWithUntil("Disable track fx", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto fx1 = *fxChain.fxByIndex(0);

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxEnabledChanged().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        fx1.disable();

        // Then
        assertTrue(!fx1.isEnabled());
        assertTrue(count == 1, "Event count wrong");
        assertTrue(*eventFx == fx1, "FX event wrong");
      });

      testWithUntil("Enable track fx", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto fx1 = *fxChain.fxByIndex(0);

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxEnabledChanged().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        fx1.enable();

        // Then
        assertTrue(fx1.isEnabled());
        assertTrue(count == 1, "Event count wrong");
        assertTrue(*eventFx == fx1, "FX event wrong");
      });

      test("Check track fx with 2 fx", [getFxChain, getTrack] {
        // Given
        auto fxChain = getFxChain();
        auto track = getTrack();

        // When
        auto fx1 = fxChain.fxByIndex(0);
        auto fx2 = fxChain.addFxByOriginalName("ReaSynth (Cockos)");

        // Then
        assertTrue(fx1.is_initialized());
        assertTrue(fx2.is_initialized());
        assertTrue(fx1->isAvailable());
        assertTrue(fx2->isAvailable());
        assertTrue(fx1->index() == 0);
        assertTrue(fx2->index() == 1);
        assertTrue(fx1->queryIndex() == (fxChain.isInputFx() ? 0x1000000 : 0));
        assertTrue(fx2->queryIndex() == (fxChain.isInputFx() ? 0x1000001 : 1));
        assertTrue(!fx1->guid().empty());
        assertTrue(!fx2->guid().empty());
        assertTrue(fx1->name() == "VST: ReaControlMIDI (Cockos)");
        assertTrue(fx2->name() == "VSTi: ReaSynth (Cockos)");
        assertTrue(fx1->chunk().startsWith("BYPASS 0 0 0"));
        assertTrue(fx1->chunk().endsWith("\nWAK 0"));
        assertTrue(fx1->tagChunk().startsWith("<VST \"VST: ReaControlMIDI (Cockos)\" reacontrolmidi"));
        assertTrue(fx1->tagChunk().endsWith("\n>"));
        assertTrue(!fx1->stateChunk().contains("<"));
        assertTrue(!fx1->stateChunk().contains(">"));
        assertTrue(fx1->fileNameWithoutExtension() == "reacontrolmidi");
        assertTrue(fx2->fileNameWithoutExtension() == "reasynth");
        assertTrue(fx1->track() == track);
        assertTrue(fx2->track() == track);
        assertTrue(fx1->isInputFx() == fxChain.isInputFx());
        assertTrue(fx2->isInputFx() == fxChain.isInputFx());
        assertTrue(fx1->chain() == fxChain);
        assertTrue(fx2->chain() == fxChain);
        assertTrue(fx1->parameterCount() == 17);
        assertTrue(fx2->parameterCount() == 15);
        assertTrue(fx1->parameters().as_blocking().count() == 17);
        assertTrue(fx1->parameterByIndex(15).isAvailable());
        assertTrue(!fx1->parameterByIndex(17).isAvailable());
        assertTrue(track.fxByQueryIndex(fxChain.isInputFx() ? 0x1000000 : 0).is_initialized());
        assertTrue(track.fxByQueryIndex(fxChain.isInputFx() ? 0x1000001 : 1).is_initialized());
        assertTrue(!track.fxByQueryIndex(fxChain.isInputFx() ? 0 : 0x1000000).is_initialized());
        assertTrue(!track.fxByQueryIndex(fxChain.isInputFx() ? 1 : 0x1000001).is_initialized());
        if (!fxChain.isInputFx()) {
          assertTrue(fxChain.firstInstrumentFx().is_initialized());
          assertTrue(fxChain.firstInstrumentFx()->index() == 1);
        }
      });

      test("Check fx parameter", [getFxChain, getTrack] {
        // Given
        auto fxChain = getFxChain();
        auto track = getTrack();
        auto fx = *fxChain.fxByIndex(0);

        // When
        auto p = fx.parameterByIndex(5);

        // Then
        assertTrue(p.isAvailable());
        assertTrue(p.name() == "Pitch Wheel");
        assertTrue(p.index() == 5);
        assertTrue(p.character() == FxParameterCharacter::Continuous);
        assertTrue(*p.clone() == p);
        assertTrue(p.track() == track);
        assertTrue(p.parameterType() == ParameterType::FX);
        assertTrue(p.formattedValue() == "0");
        assertTrue(p.normalizedValue() == 0.5);
        assertTrue(p.reaperValue() == 0.5);
        assertTrue(p.formatNormalizedValue(p.normalizedValue()) == "0");
        assertTrue(p.fx() == fx);
        assertTrue(p.stepSize() == -1);
      });

      test("Check fx presets", [getFxChain] {
        // Given
        auto fxChain = getFxChain();
        auto fx = *fxChain.fxByIndex(0);

        // When

        // Then
        // TODO Preset count could be > 0 on some installations
        assertTrue(fx.presetCount() == 0);
        assertTrue(fx.presetName().empty());
        assertTrue(fx.presetIsDirty());
      });

      testWithUntil("Set fx parameter value", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto fx = *fxChain.fxByIndex(1);
        auto p = fx.parameterByIndex(5);

        // When
        optional<FxParameter> eventFxParameter;
        int count = 0;
        Reaper::instance().fxParameterValueChanged().take_until(testIsOver).subscribe([&](FxParameter f) {
          count++;
          eventFxParameter = f;
        });
        p.setNormalizedValue(0.3);

        // Then
        auto lastTouchedFxParam = Reaper::instance().lastTouchedFxParameter();
        // FIXME We should also make this work for input FX!
        if (!fxChain.isInputFx()) {
          assertTrue(lastTouchedFxParam.is_initialized(), "No last touched FX param");
          assertTrue(*lastTouchedFxParam == p, "Last touched FX param not correct");
        }
        assertTrue(p.formattedValue() == "-4.44");
        assertTrue(p.normalizedValue() == 0.30000001192092896);
        assertTrue(p.reaperValue() == 0.30000001192092896);
        assertTrue(p.formatNormalizedValue(p.normalizedValue()) == "-4.44 dB");
        // TODO 1 invocation would be better than 2
        assertTrue(count == 2, "Event count wrong (maybe 1 instead of 2, that would be an improvement?)");
        assertTrue(*eventFxParameter == p, "FX parameter event wrong");
      });

      testWithUntil("Move FX", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto midiFx = *fxChain.fxByIndex(0);
        auto synthFx = *fxChain.fxByIndex(1);

        // When
        optional<Track> eventTrack;
        int count = 0;
        Reaper::instance().fxReordered().take_until(testIsOver).subscribe([&](Track t) {
          count++;
          eventTrack = t;
        });
        fxChain.moveFx(synthFx, 0);

        // Then
        assertTrue(midiFx.index() == 1);
        assertTrue(synthFx.index() == 0);
        // TODO Detect such a programmatic FX move as well (maybe by hooking into HelperControlSurface::updateMediaTrackPositions)
        assertTrue(count == 0, "Event count wrong, maybe 1, that would be an improvement");
        // TODO Add FxChain.track()
        assertTrue(!eventTrack.is_initialized(), "Track event wrong, maybe an improvement");
      });

      testWithUntil("Remove FX", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto synthFx = *fxChain.fxByIndex(0);
        auto midiFx = *fxChain.fxByIndex(1);

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxRemoved().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        fxChain.removeFx(synthFx);

        // Then
        assertTrue(!synthFx.isAvailable());
        assertTrue(midiFx.isAvailable());
        assertTrue(midiFx.index() == 0);
        midiFx.invalidateIndex();
        assertTrue(count == 1, "Event count wrong");
        assertTrue(*eventFx == synthFx, "FX event wrong");
      });

      testWithUntil("Add FX by chunk", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto fxChunk = R"foo(BYPASS 0 0 0
  <VST "VSTi: ReaSynth (Cockos)" reasynth.dll 0 "" 1919251321
  eXNlcu9e7f4AAAAAAgAAAAEAAAAAAAAAAgAAAAAAAAA8AAAAAAAAAAAAEAA=
  776t3g3wrd6mm8Q7F7fROgAAAAAAAAAAAAAAAM5NAD/pZ4g9AAAAAAAAAD8AAIA/AACAPwAAAD8AAAAA
  AAAQAAAA
  >
  FLOATPOS 0 0 0 0
  FXID {5FF5FB09-9102-4CBA-A3FB-3467BA1BFE5D}
  WAK 0)foo";

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxAdded().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        auto synthFx = fxChain.addFxOfChunk(fxChunk);

        // Then
        assertTrue(synthFx.is_initialized());
        assertTrue(synthFx->index() == 1);
        assertTrue(synthFx->guid() == "5FF5FB09-9102-4CBA-A3FB-3467BA1BFE5D");
        assertTrue(synthFx->parameterByIndex(5).formattedValue() == "-6.00");
        // TODO Detect such a programmatic FX add as well (maybe by hooking into HelperControlSurface::updateMediaTrackPositions)
        assertTrue(count == 0, "Event count wrong, could be an improvement");
        assertTrue(!eventFx.is_initialized(), "FX event wrong, could be an improvement");
      });

      testWithUntil("Set fx chunk", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto midiFx = *fxChain.fxByIndex(0);
        auto synthFx = *fxChain.fxByIndex(1);
        auto synthFxGuidBefore = synthFx.guid();

        // When
        synthFx.setChunk(midiFx.chunk().content().to_string().c_str());

        // Then
        assertTrue(synthFx.guid() == synthFxGuidBefore, "GUIDs have changed");
        assertTrue(synthFx.name() == "VST: ReaControlMIDI (Cockos)");
        assertTrue(midiFx.index() == 0);
        assertTrue(synthFx.index() == 1);
      });

      test("Set fx tag chunk", [getFxChain] {
        // Given
        auto fxChain = getFxChain();
        auto midiFx1 = *fxChain.fxByIndex(0);
        auto midiFx2 = *fxChain.fxByIndex(1);
        auto fxTagChunk = R"foo(<VST "VSTi: ReaSynth (Cockos)" reasynth.dll 0 "" 1919251321
  eXNlcu9e7f4AAAAAAgAAAAEAAAAAAAAAAgAAAAAAAAA8AAAAAAAAAAAAEAA=
  776t3g3wrd6mm8Q7F7fROgAAAAAAAAAAAAAAAM5NAD/pZ4g9AAAAAAAAAD8AAIA/AACAPwAAAD8AAAAA
  AAAQAAAA
  >)foo";

        // When
        midiFx2.setTagChunk(fxTagChunk);

        // Then
        assertTrue(midiFx2.index() == 1);
        assertTrue(midiFx2.name() == "VSTi: ReaSynth (Cockos)");
        assertTrue(midiFx1.index() == 0);
        assertTrue(midiFx1.name() == "VST: ReaControlMIDI (Cockos)");
      });

      testWithUntil("Set fx state chunk", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto midiFx = *fxChain.fxByIndex(0);
        auto synthFx = *fxChain.fxByIndex(1);
        synthFx.parameterByIndex(5).setNormalizedValue(0);
        assertTrue(synthFx.parameterByIndex(5).formattedValue() != "-6.00");
        auto fxStateChunk = R"foo(eXNlcu9e7f4AAAAAAgAAAAEAAAAAAAAAAgAAAAAAAAA8AAAAAAAAAAAAEAA=
  776t3g3wrd6mm8Q7F7fROgAAAAAAAAAAAAAAAM5NAD/pZ4g9AAAAAAAAAD8AAIA/AACAPwAAAD8AAAAA
  AAAQAAAA)foo";

        // When
        synthFx.setStateChunk(fxStateChunk);

        // Then
        assertTrue(synthFx.index() == 1);
        assertTrue(synthFx.name() == "VSTi: ReaSynth (Cockos)");
        assertTrue(synthFx.parameterByIndex(5).formattedValue() == "-6.00");
        assertTrue(midiFx.index() == 0);
        assertTrue(midiFx.name() == "VST: ReaControlMIDI (Cockos)");
      });

      testWithUntil("Set fx chain chunk", [getFxChain, getTrack](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();
        auto track = getTrack();
        auto otherFxChain = fxChain.isInputFx() ? track.normalFxChain() : track.inputFxChain();
        string prefix = fxChain.isInputFx() ? "<FXCHAIN" : "<FXCHAIN_REC";
        auto fxChainChunk = prefix + R"foo(
  SHOW 0
  LASTSEL 0
  DOCKED 0
  BYPASS 0 0 0
  <VST "VST: ReaControlMIDI (Cockos)" reacontrolmidi.dll 0 "" 1919118692
  ZG1jcu5e7f4AAAAAAAAAAOYAAAABAAAAAAAQAA==
  /////wAAAAAAAAAAAAAAAAkAAAAMAAAAAQAAAP8/AAAAIAAAACAAAAAAAAA1AAAAQzpcVXNlcnNcYmtsdW1cQXBwRGF0YVxSb2FtaW5nXFJFQVBFUlxEYXRhXEdNLnJlYWJhbmsAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAYAAABNYWpvcgANAAAAMTAyMDM0MDUwNjA3AAEAAAAAAAAAAAAAAAAKAAAA
  DQAAAAEAAAAAAAAAAAAAAAAAAAA=
  AAAQAAAA
  >
  FLOATPOS 0 0 0 0
  FXID {80028901-3762-477F-BE48-EA8324C178AA}
  WAK 0
  BYPASS 0 0 0
  <VST "VSTi: ReaSynth (Cockos)" reasynth.dll 0 "" 1919251321
  eXNlcu9e7f4AAAAAAgAAAAEAAAAAAAAAAgAAAAAAAAA8AAAAAAAAAAAAEAA=
  776t3g3wrd6mm8Q7F7fROgAAAAAAAAAAAAAAAM5NAD/pZ4g9AAAAAAAAAD8AAIA/AACAPwAAAD8AAAAA
  AAAQAAAA
  >
  FLOATPOS 0 0 0 0
  FXID {5FF5FB09-9102-4CBA-A3FB-3467BA1BFEAA}
  WAK 0
  >)foo";

        // When
        otherFxChain.setChunk(fxChainChunk.c_str());

        // Then
        assertTrue(otherFxChain.fxCount() == 2);
        assertTrue(fxChain.fxCount() == 2, "FX not in normal chain anymore");
      });

      test("Query fx floating window", [getFxChain] {
        // Given
        auto fxChain = getFxChain();
        auto fx = *fxChain.fxByIndex(0);

        // When

        // Then
        assertTrue(fx.floatingWindow() == nullptr);
        assertTrue(!fx.windowIsOpen());
        assertTrue(!fx.windowHasFocus());
        if (!fxChain.isInputFx()) {
          assertTrue(!Reaper::instance().focusedFx().is_initialized());
        }
      });

      test("Show fx in floating window", [getFxChain] {
        // Given
        auto fxChain = getFxChain();
        auto fx = *fxChain.fxByIndex(0);

        // When
        fxChain.fxByIndex(0)->showInFloatingWindow();

        // Then
        assertTrue(fx.floatingWindow() != nullptr);
        assertTrue(fx.windowIsOpen());
        assertTrue(fx.windowHasFocus(), "Window has no focus");
        if (!fxChain.isInputFx()) {
          const auto focusedFx = Reaper::instance().focusedFx();
          // TODO This is not reliable! After REAPER start no focused Fx can be found!
//            assertTrue(focusedFx.is_initialized(), "Focused FX not found");
//            assertTrue(*focusedFx == fx, "Wrong focused fx");
        }
      });

      testWithUntil("Add track JS fx by original name", [getFxChain](auto testIsOver) {
        // Given
        auto fxChain = getFxChain();

        // When
        optional<Fx> eventFx;
        int count = 0;
        Reaper::instance().fxAdded().take_until(testIsOver).subscribe([&](Fx f) {
          count++;
          eventFx = f;
        });
        auto fx = fxChain.addFxByOriginalName("phaser");

        // Then
        assertTrue(fx.is_initialized());
        assertTrue(fxChain.fxCount() == 3);
        assertTrue(fxChain.fxs().as_blocking().count() == 3);
        assertTrue(fxChain.fxByIndex(2).is_initialized());
        assertTrue(*fxChain.fxByIndex(2) == *fx);
        assertTrue(fxChain.lastFx().is_initialized());
        assertTrue(*fxChain.lastFx() == *fx);
        assertTrue(!fx->guid().empty());
        assertTrue(fxChain.fxByGuid(fx->guid()).isAvailable());
        assertTrue(!fxChain.fxByGuidAndIndex("bla", 0).isAvailable());
        assertTrue(fxChain.firstFxByName("ReaControlMIDI (Cockos)").is_initialized());
        assertTrue(*fxChain.firstFxByName("phaser") == *fx);
        assertTrue(count == 1, "Event count wrong");
        assertTrue(*eventFx == fx, "FX event wrong");
      });

      test("Query track JS fx by index", [getFxChain, getTrack] {
        // Given
        auto fxChain = getFxChain();
        auto track = getTrack();

        // When
        auto fx = fxChain.fxByIndex(2);

        // Then
        assertTrue(fx.is_initialized());
        assertTrue(fx->isAvailable());
        assertTrue(fx->index() == 2);
        assertTrue(fx->queryIndex() == (fxChain.isInputFx() ? 0x1000002 : 2));
        assertTrue(!fx->guid().empty());
        assertTrue(fx->name() == "JS: phaser");
        assertTrue(fx->chunk().startsWith("BYPASS 0 0 0"));
        assertTrue(fx->chunk().endsWith("\nWAK 0"));
        auto tagChunk = fx->tagChunk().content().to_string();
        assertTrue(fx->tagChunk().startsWith("<JS phaser \"\""));
        assertTrue(fx->tagChunk().endsWith("\n>"));
        assertTrue(!fx->stateChunk().contains("<"));
        assertTrue(!fx->stateChunk().contains(">"));
        assertTrue(fx->fileNameWithoutExtension() == "phaser");
        assertTrue(fx->track() == track);
        assertTrue(fx->isInputFx() == fxChain.isInputFx());
        assertTrue(fx->chain() == fxChain);
        assertTrue(fx->parameterCount() == 7);
        assertTrue(fx->parameters().as_blocking().count() == 7);
        assertTrue(fx->parameterByIndex(6).isAvailable());
        assertTrue(!fx->parameterByIndex(7).isAvailable());
      });
    };

    fxTests(
        [this] { return firstTrack(); },
        [this] { return firstTrack().normalFxChain(); }
    );

    fxTests(
        [this] { return secondTrack(); },
        [this] { return secondTrack().inputFxChain(); }
    );

    testWithUntil("Insert track at", [](auto testIsOver) {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackAdded().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      auto newTrack = project.insertTrackAt(1);
      newTrack.setName("Inserted track");

      // Then
      assertTrue(project.trackCount() == 4, "Track count not increased");
      assertTrue(newTrack.index() == 1, "Track index wrong");
      assertTrue(newTrack.name() == "Inserted track", "Track name wrong");
      assertTrue(secondTrack.index() == 2, "Tracks after not correctly moved");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == newTrack, "Track event wrong");
    });

    test("Query MIDI input devices", [] {
      // Given

      // When
      Reaper::instance().midiInputDevices();
      Reaper::instance().midiInputDeviceById(0).isAvailable();

      // Then
    });

    test("Query MIDI output devices", [] {
      // Given

      // When
      Reaper::instance().midiOutputDevices();
      Reaper::instance().midiOutputDeviceById(0).isAvailable();

      // Then
    });

    testWithUntil("Use undoable", [](auto testIsOver) {
      // Given
      auto t = firstTrack();

      // When
      optional<Track> eventTrack;
      int count = 0;
      Reaper::instance().trackNameChanged().take_until(testIsOver).subscribe([&](Track t) {
        count++;
        eventTrack = t;
      });
      Reaper::instance().currentProject().undoable("ReaPlus integration test operation", [t]() mutable {
        t.setName("Renamed");
      });
      const auto label = Reaper::instance().currentProject().labelOfLastUndoableAction();

      // Then
      assertTrue(t.name() == "Renamed", "Undoable operation was not executed");
      assertTrue(label && *label == "ReaPlus integration test operation", "Label was wrong");
      assertTrue(count == 1, "Event count wrong");
      assertTrue(*eventTrack == t, "Track event wrong");
    });

    test("Undo", [] {
      // Given
      auto t = firstTrack();

      // When
      Reaper::instance().currentProject().undo();
      const auto label = Reaper::instance().currentProject().labelOfNextRedoableAction();

      // Then
      assertTrue(firstTrack().name().empty(), "Undo was not executed or undoable didn't really work");
      assertTrue(label && *label == "ReaPlus integration test operation", "Label was wrong");
    });

    test("Redo", [] {
      // Given
      auto t = firstTrack();

      // When
      Reaper::instance().currentProject().redo();

      // Then
      assertTrue(firstTrack().name() == "Renamed", "Redo didn't work");
    });

    test("Get REAPER window", [] {
      // Given

      // When
      auto result = Reaper::instance().mainWindow();

      // Then
      assertTrue(result == reaper::GetMainHwnd(), "Wrong main window");
    });

    test("Mark project as dirty", [] {
      // Given

      // When
      Reaper::instance().currentProject().markAsDirty();

      // Then
    });

    test("Get project tempo", [] {
      // Given

      // When
      const auto tempo = Reaper::instance().currentProject().tempo();

      // Then
      assertTrue(tempo.bpm() == 120.0);
      assertTrue(tempo.normalizedValue() == 119.0 / 959);
    });

    testWithUntil("Set project tempo", [](auto testIsOver) {
      // Given

      // When
      int count = 0;
      Reaper::instance().masterTempoChanged().take_until(testIsOver).subscribe([&](bool) {
        count++;
      });
      Reaper::instance().currentProject().setTempo(130.0, false);

      // Then
      assertTrue(Reaper::instance().currentProject().tempo().bpm() == 130.0);
      // TODO There should be only one event invocation
      assertTrue(count == 2, "Event count wrong, but could be 1 which would be more correct");
    });

    test("Show message box", [] {
      // Given

      // When
      auto result = Reaper::instance().showMessageBox("Tests are finished", "ReaPlus", MessageBoxType::Ok);

      // Then
      assertTrue(result == MessageBoxResult::Ok);
    });

  }

  void ReaPlusIntegrationTest::test(const std::string& name, std::function<void(void)> code) {
    testInternal(name, [code](observable<bool>) {
      code();
      return observable<>::just(true);
    });
  }

  void ReaPlusIntegrationTest::testWithUntil(const std::string& name,
      std::function<void(rxcpp::observable<bool>)> code) {
    testInternal(name, [code](observable<bool> testIsOver) {
      code(testIsOver);
      return observable<>::just(true);
    });
  }
  void ReaPlusIntegrationTest::testAndWait(const std::string& name,
      std::function<rxcpp::observable<bool>(void)> code) {
    testInternal(name, [code](observable<bool>) {
      return code();
    });
  }
  void ReaPlusIntegrationTest::testInternal(const std::string& name,
      std::function<rxcpp::observable<bool>(rxcpp::observable<bool>)> code) {
    stepQueue_.push(TestStep(name, std::move(code)));
  }

  void ReaPlusIntegrationTest::processSuccess() {
    log("\nSuccessful");
  }

  void ReaPlusIntegrationTest::processFailure(const std::exception& e) {
    log("\nFailed");
    if (std::strlen(e.what()) > 0) {
      log(":");
      log(e.what());
    }
  }

  void ReaPlusIntegrationTest::logHeading(const std::string& name) {
    log("\n\n## ");
    log(name);
  }

  void ReaPlusIntegrationTest::execute() {
    Reaper::instance().clearConsole();
    stepQueue_ = {};
    log("# Testing ReaPlus");
    try {
      tests();
      executeNextStep();
    } catch (const std::exception&) {
      log("\n\nFailure while building test steps");
    }
  }
  void ReaPlusIntegrationTest::executeNextStep() {
    if (stepQueue_.empty()) {
      log("\n\nAll tests successfully executed");
    } else {
      const auto& step = stepQueue_.front();
      logHeading(step.getName());
      rxcpp::subjects::subject<bool> testIsOver;
      const auto op = step.getOperation();
      try {
        const auto testFinished = op(testIsOver.get_observable()).take(1);
        testFinished.subscribe([this, testIsOver](bool successful) {
          testIsOver.get_subscriber().on_next(true);
          if (successful) {
            processSuccess();
            stepQueue_.pop();
            Reaper::instance().executeLaterInMainThread([this] {
              executeNextStep();
            });
          } else {
            log("\nAsync result different than expected");
          }
        });
      } catch (const std::exception& e) {
        testIsOver.get_subscriber().on_next(true);
        processFailure(e);
      }
    }
  }

  void ReaPlusIntegrationTest::log(const std::string& msg) {
    Reaper::instance().showConsoleMessage(msg);
  }

  void ReaPlusIntegrationTest::assertTrue(bool expression, const std::string& errorMsg) {
    if (!expression) {
      throw std::logic_error(errorMsg);
    }
  }

  Track ReaPlusIntegrationTest::firstTrack() {
    auto project = Reaper::instance().currentProject();
    return *project.firstTrack();
  }

  Track ReaPlusIntegrationTest::secondTrack() {
    auto project = Reaper::instance().currentProject();
    return *project.trackByIndex(1);
  }

  TestStep::TestStep(const std::string& name, TestStep::Operation operation)
      : name_(name), operation_(operation) {
  }
  std::string TestStep::getName() const {
    return name_;
  }
  TestStep::Operation TestStep::getOperation() const {
    return operation_;
  }
}