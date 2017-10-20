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

using boost::none;
using boost::optional;
using std::string;

namespace reaplus {
  void ReaPlusIntegrationTest::tests() const {
    test("Create empty project in new tab", [] {
      // Given
      const auto currentProjectBefore = Reaper::instance().currentProject();
      const int projectCountBefore = Reaper::instance().projectCount();

      // When
      const auto newProject = Reaper::instance().createEmptyProjectInNewTab();

      // Then
      assertTrue(currentProjectBefore == currentProjectBefore, "Project comparison broken");
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
    });

    test("Add track", [] {
      // Given
      auto project = Reaper::instance().currentProject();

      // When
      const auto newTrack = project.addTrack();

      // Then
      assertTrue(project.trackCount() == 1, "Track count not increased");
      assertTrue(newTrack.index() == 0, "Track index wrong");
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
      assertTrue(trackName == "", "Wrong name reported");
    });

    test("Set track name", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setName("Foo Bla");

      // Then
      assertTrue(track.name() == "Foo Bla", "Name not correctly set");
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
    });

    test("Set track recording input 1", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setRecordingInput(MidiRecordingInput::fromDeviceAndChannel(4, 5));

      // Then
      auto recInput = track.recordingInput();
      assertTrue(recInput != nullptr, "Returned empty recording input");
      assertTrue(recInput->type() == RecordingInputType::Midi, "Returned wrong recording input type");
      auto& midiRecInput = dynamic_cast<MidiRecordingInput&>(*recInput);
      assertTrue(midiRecInput.channel() == 5, "Returned wrong channel");
      assertTrue(midiRecInput.device()->id() == 4, "Returned wrong device");
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

    test("Set track volume", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setVolume(0.25);

      // Then
      auto volume = track.volume();
      assertTrue(volume.reaperValue() == 0.031588093366685013, "Wrong REAPER value returned");
      assertTrue(volume.db() == -30.009531739774296, "Wrong db returned");
      assertTrue(volume.normalizedValue() == 0.25000000000003497, "Wrong normalized value returned");
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

    test("Set track pan", [] {
      // Given
      auto track = firstTrack();

      // When
      track.setPan(0.25);

      // Then
      auto pan = track.pan();
      assertTrue(pan.reaperValue() == -0.5, "Wrong REAPER value returned");
      assertTrue(pan.normalizedValue() == 0.25, "Wrong normalized value returned");
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

    test("Select track", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto track = firstTrack();
      auto track2 = *project.trackByIndex(2);

      // When
      track.select();
      track2.select();

      // Then
      assertTrue(track.isSelected(), "Track was not selected");
      assertTrue(project.selectedTrackCount() == 2);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.firstSelectedTrack()->index() == 0);
      assertTrue(project.selectedTracks().as_blocking().count() == 2);
    });

    test("Unselect track", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto track = firstTrack();

      // When
      track.unselect();

      // Then
      assertTrue(!track.isSelected(), "Track was not unselected");
      assertTrue(project.selectedTrackCount() == 1);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.firstSelectedTrack()->index() == 2);
      assertTrue(project.selectedTracks().as_blocking().count() == 1);
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

      // Then
      assertTrue(!isArmed, "Wrong value returned");
    });

    test("Arm track in normal mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.arm();

      // Then
      assertTrue(track.isArmed(), "Track was not armed");
      assertTrue(!track.hasAutoArmEnabled(), "Track is not in normal mode anymore");
    });

    test("Disarm track in normal mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.disarm();

      // Then
      assertTrue(!track.isArmed(), "Track was not disarmed");
      assertTrue(!track.hasAutoArmEnabled(), "Track is not in normal mode anymore");
    });

    test("Enable track auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.enableAutoArm();

      // Then
      assertTrue(track.hasAutoArmEnabled(), "Track is still in normal mode");
      assertTrue(!track.isArmed(), "Track is suddenly armed");
    });

    test("Arm track in auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.arm();

      // Then
      assertTrue(track.isArmed(), "Track was not armed");
      assertTrue(track.hasAutoArmEnabled(), "Track is not in auto-arm mode anymore");
    });

    test("Disarm track in auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.disarm();

      // Then
      assertTrue(!track.isArmed(), "Track was not disarmed");
      assertTrue(track.hasAutoArmEnabled(), "Track is not in auto-arm mode anymore");
    });

    test("Disable track auto-arm mode", [] {
      // Given
      auto track = firstTrack();

      // When
      track.disableAutoArm();

      // Then
      assertTrue(!track.hasAutoArmEnabled(), "Track is still in auto-arm mode");
      assertTrue(!track.isArmed(), "Track is suddenly armed");
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
    });

    test("Select track exclusively", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);
      auto thirdTrack = *project.trackByIndex(2);
      firstTrack.unselect();
      secondTrack.select();
      thirdTrack.select();

      // When
      firstTrack.selectExclusively();

      // Then
      assertTrue(firstTrack.isSelected(), "First track has not been selected");
      assertTrue(!secondTrack.isSelected(), "Second track is still selected");
      assertTrue(!thirdTrack.isSelected(), "Third track is still selected");
      assertTrue(project.selectedTrackCount() == 1);
      assertTrue(project.firstSelectedTrack().is_initialized());
      assertTrue(project.selectedTracks().as_blocking().count() == 1);
    });

    test("Remove track", [] {
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
      project.removeTrack(firstTrack);

      // Then
      assertTrue(project.trackCount() == trackCountBefore - 1, "Track count still same as before");
      assertTrue(!firstTrack.isAvailable(), "Removed track still available");
      assertTrue(secondTrack.index() == 0, "Index of track after removed track not invalidated");
      assertTrue(secondTrack.guid() == secondTrackGuid, "GUID of track after removed track has changed");
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

    test("Check track send", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);
      auto thirdTrack = project.addTrack();

      // When
      auto sendToSecondTrack = firstTrack.sendByTargetTrack(secondTrack);
      auto sendToThirdTrack = firstTrack.addSendTo(thirdTrack);
      sendToThirdTrack.setVolume(0.25);

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
      assertTrue(sendToThirdTrack.volume().db() == -30.009531739774296);
    });

    test("Query action", [] {
      // Given
      firstTrack().selectExclusively();
      assertTrue(!firstTrack().isMuted());

      // When
      auto toggleAction = Reaper::instance().mainSection().actionByCommandId(6);
      auto normalAction = Reaper::instance().mainSection().actionByCommandId(41075);

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
    });

    test("Invoke action", [] {
      // Given
      auto action = Reaper::instance().mainSection().actionByCommandId(6);

      // When
      action.invoke();

      // Then
      assertTrue(action.isOn());
      assertTrue(firstTrack().isMuted());
    });

    test("Unmute track", [] {
      // Given
      auto track = firstTrack();

      // When
      firstTrack().unmute();

      // Then
      assertTrue(!track.isMuted());
    });

    test("Mute track", [] {
      // Given
      auto track = firstTrack();

      // When
      firstTrack().mute();

      // Then
      assertTrue(track.isMuted());
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

    auto fxTests = [this](Track track, FxChain fxChain) {
      test("Query fx chain", [&fxChain] {
        // Given

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


      test("Add track fx by original name", [&fxChain] {
        // Given

        // When
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
      });

      test("Check track fx with 1 fx", [&fxChain, &track] {
        // Given

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

      test("Disable track fx", [&fxChain, &track] {
        // Given
        auto fx1 = *fxChain.fxByIndex(0);

        // When
        fx1.disable();

        // Then
        assertTrue(!fx1.isEnabled());
      });

      test("Enable track fx", [&fxChain, &track] {
        // Given
        auto fx1 = *fxChain.fxByIndex(0);

        // When
        fx1.enable();

        // Then
        assertTrue(fx1.isEnabled());
      });

      test("Check track fx with 2 fx", [&fxChain, &track] {
        // Given

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

      test("Check fx parameter", [&fxChain, &track] {
        // Given
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

      test("Check fx presets", [&fxChain, &track] {
        // Given
        auto fx = *fxChain.fxByIndex(0);

        // When

        // Then
        // TODO Preset count could be > 0 on some installations
        assertTrue(fx.presetCount() == 0);
        assertTrue(fx.presetName().empty());
        assertTrue(fx.presetIsDirty());
      });

      test("Set fx parameter value", [&fxChain] {
        // Given
        auto fx = *fxChain.fxByIndex(1);
        auto p = fx.parameterByIndex(5);

        // When
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

      });

      test("Move FX", [&fxChain] {
        auto midiFx = *fxChain.fxByIndex(0);
        auto synthFx = *fxChain.fxByIndex(1);

        // When
        fxChain.moveFx(synthFx, 0);

        // Then
        assertTrue(midiFx.index() == 1);
        assertTrue(synthFx.index() == 0);
      });

      test("Remove FX", [&fxChain] {
        // Given
        auto synthFx = *fxChain.fxByIndex(0);
        auto midiFx = *fxChain.fxByIndex(1);

        // When
        fxChain.removeFx(synthFx);

        // Then
        assertTrue(!synthFx.isAvailable());
        assertTrue(midiFx.isAvailable());
        assertTrue(midiFx.index() == 0);
        midiFx.invalidateIndex();
      });

      test("Add FX by chunk", [&fxChain] {
        // Given
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
        auto synthFx = fxChain.addFxOfChunk(fxChunk);

        // Then
        assertTrue(synthFx.is_initialized());
        assertTrue(synthFx->index() == 1);
        assertTrue(synthFx->guid() == "5FF5FB09-9102-4CBA-A3FB-3467BA1BFE5D");
        assertTrue(synthFx->parameterByIndex(5).formattedValue() == "-6.00");
      });

      test("Set fx chunk", [&fxChain] {
        // Given
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

      test("Set fx tag chunk", [&fxChain] {
        // Given
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

      test("Set fx state chunk", [&fxChain] {
        // Given
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

      test("Set fx chain chunk", [&fxChain, &track] {
        // Given
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

      test("Query fx floating window", [&fxChain] {
        // Given
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


      test("Show fx in floating window", [&fxChain] {
        // Given
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

      test("Add track JS fx by original name", [&fxChain] {
        // Given

        // When
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
      });

      test("Add track JS fx by original name", [&fxChain, &track] {
        // Given

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

    {
      fxTests(firstTrack(), firstTrack().normalFxChain());
      auto secondTrack = *Reaper::instance().currentProject().trackByIndex(1);
      fxTests(secondTrack, secondTrack.inputFxChain());
    }

    test("Insert track at", [] {
      // Given
      auto project = Reaper::instance().currentProject();
      auto firstTrack = *project.trackByIndex(0);
      auto secondTrack = *project.trackByIndex(1);

      // When
      auto newTrack = project.insertTrackAt(1);
      newTrack.setName("Inserted track");

      // Then
      assertTrue(project.trackCount() == 4, "Track count not increased");
      assertTrue(newTrack.index() == 1, "Track index wrong");
      assertTrue(newTrack.name() == "Inserted track", "Track name wrong");
      assertTrue(secondTrack.index() == 2, "Tracks after not correctly moved");
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

    test("Show message box", [] {
      // Given

      // When
      auto result = Reaper::instance().showMessageBox("Tests are finished", "ReaPlus", MessageBoxType::Ok);

      // Then
      assertTrue(result == MessageBoxResult::Ok);
    });

  }

  void ReaPlusIntegrationTest::test(const std::string& name, std::function<void(void)> code) const {
    log("\n\n## ");
    log(name);
    try {
      code();
      log("\nSuccessful");
    } catch (const std::exception& e) {
      log("\nFailed");
      if (std::strlen(e.what()) > 0) {
        log(":");
        log(e.what());
      }
      if (stopOnFailure_) {
        throw e;
      }
    }
  }

  ReaPlusIntegrationTest::ReaPlusIntegrationTest(bool stopOnFailure) : stopOnFailure_(stopOnFailure) {
  }

  void ReaPlusIntegrationTest::execute() const {
    Reaper::instance().clearConsole();
    log("# Testing ReaPlus");
    try {
      tests();
      if (stopOnFailure_) {
        log("\n\nAll tests successfully executed");
      }
    } catch (const std::exception&) {
      log("\n\nFurther execution of tests stopped");
    }
  }

  void ReaPlusIntegrationTest::log(const std::string& msg) const {
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
}