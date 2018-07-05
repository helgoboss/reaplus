#pragma once

#include <functional>
#include <unordered_map>
#include <set>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <mutex>
#include "reaper_plugin.h"
#include "rxcpp/rx.hpp"
#include "util/rx-relaxed-runloop.hpp"
#include "Project.h"
#include "Fx.h"
#include "FxParameter.h"
#include "Parameter.h"
#include "Track.h"
#include "concurrentqueue.h"

namespace reaplus {

  class FxParameter;

  class Track;

  class TrackSend;

  class Reaper;

  struct TrackData {
    double volume;
    double pan;
    bool selected;
    bool mute;
    bool solo;
    bool recarm;
    int number;
    int recmonitor;
    int recinput;
    std::string guid;
  };

  struct FxChainPair {
    std::set<std::string> inputFxGuids;
    std::set<std::string> outputFxGuids;
  };

  class HelperControlSurface : public IReaperControlSurface {
    friend class Reaper;
    friend class Track;
  private:
    class Guard {
    public:
      ~Guard();
    };

    enum class State {
      Normal,
      PropagatingTrackSetChanges
    };

    static std::unique_ptr<HelperControlSurface> INSTANCE;
    static constexpr int FAST_COMMAND_BUFFER_SIZE = 100;
    int numTrackSetChangesLeftToBePropagated_ = 0;
    rxcpp::subjects::subject<FxParameter> fxParameterValueChangedSubject_;
    rxcpp::subjects::subject<FxParameter> fxParameterTouchedSubject_;
    bool fxHasBeenTouchedJustAMomentAgo_ = false;
    rxcpp::subjects::subject<Track> trackVolumeChangedSubject_;
    rxcpp::subjects::subject<Track> trackVolumeTouchedSubject_;
    rxcpp::subjects::subject<Track> trackPanChangedSubject_;
    rxcpp::subjects::subject<Track> trackPanTouchedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendVolumeChangedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendVolumeTouchedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendPanChangedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendPanTouchedSubject_;
    rxcpp::subjects::subject<Track> trackAddedSubject_;
    rxcpp::subjects::subject<Track> trackRemovedSubject_;
    rxcpp::subjects::subject<Project> tracksReorderedSubject_;
    rxcpp::subjects::subject<Track> trackNameChangedSubject_;
    rxcpp::subjects::subject<Track> trackInputChangedSubject_;
    rxcpp::subjects::subject<Track> trackInputMonitoringChangedSubject_;
    rxcpp::subjects::subject<Track> trackArmChangedSubject_;
    rxcpp::subjects::subject<Track> trackMuteChangedSubject_;
    rxcpp::subjects::subject<Track> trackMuteTouchedSubject_;
    rxcpp::subjects::subject<Track> trackSoloChangedSubject_;
    rxcpp::subjects::subject<Track> trackSelectedChangedSubject_;
    rxcpp::subjects::subject<Fx> fxAddedSubject_;
    rxcpp::subjects::subject<Fx> fxRemovedSubject_;
    rxcpp::subjects::subject<Fx> fxEnabledChangedSubject_;
    rxcpp::subjects::subject<Track> fxReorderedSubject_;
    rxcpp::subjects::subject<bool> masterTempoChangedSubject_;
    rxcpp::subjects::subject<bool> masterTempoTouchedSubject_;
    rxcpp::subjects::subject<bool> masterPlayrateChangedSubject_;
    rxcpp::subjects::subject<bool> masterPlayrateTouchedSubject_;
    rxcpp::subjects::subject<bool> mainThreadIdleSubject_;
    rxcpp::subjects::behavior<Project> activeProjectBehavior_;
    using TrackDataMap = std::unordered_map<MediaTrack*, TrackData>;
    std::unordered_map<ReaProject*, TrackDataMap> trackDataByMediaTrackByReaProject_;
    std::unordered_map<MediaTrack*, FxChainPair> fxChainPairByMediaTrack_;
    rxcpp::schedulers::relaxed_run_loop mainThreadRunLoop_;
    rxcpp::observe_on_one_worker mainThreadCoordination_ =
        rxcpp::observe_on_one_worker(rxcpp::schedulers::make_relaxed_run_loop(mainThreadRunLoop_));
    rxcpp::observe_on_one_worker::coordinator_type
        mainThreadCoordinator_ = mainThreadCoordination_.create_coordinator();
    moodycamel::ConcurrentQueue<std::function<void(void)>> fastCommandQueue_;
    std::array<std::function<void(void)>, FAST_COMMAND_BUFFER_SIZE> fastCommandBuffer_;

  public:
    ~HelperControlSurface() override;

    void SetSurfacePan(MediaTrack* trackid, double pan) override;

    void SetSurfaceVolume(MediaTrack* trackid, double volume) override;

    const char* GetTypeString() override;

    const char* GetDescString() override;

    const char* GetConfigString() override;

    void SetTrackListChange() override;

    void Run() override;

    int Extended(int call, void* parm1, void* parm2, void* parm3) override;

    void SetTrackTitle(MediaTrack* trackid, const char* title) override;

    void SetSurfaceMute(MediaTrack* trackid, bool mute) override;

    void SetSurfaceSelected(MediaTrack* trackid, bool selected) override;

    void SetSurfaceSolo(MediaTrack* trackid, bool solo) override;

    void SetSurfaceRecArm(MediaTrack* trackid, bool recarm) override;

  protected:

    static void init();

    static HelperControlSurface& instance();

    static void destroyInstance();

    rxcpp::observable<Parameter*> parameterValueChangedUnsafe() const;

    rxcpp::observable<Parameter*> parameterTouchedUnsafe() const;

    rxcpp::observable<FxParameter> fxParameterValueChanged() const;

    rxcpp::observable<FxParameter> fxParameterTouched() const;

    rxcpp::observable<Track> trackVolumeChanged() const;

    rxcpp::observable<Track> trackVolumeTouched() const;

    rxcpp::observable<Track> trackPanChanged() const;

    rxcpp::observable<Track> trackNameChanged() const;

    rxcpp::observable<Track> trackInputMonitoringChanged() const;

    rxcpp::observable<Track> trackArmChanged() const;

    rxcpp::observable<Track> trackMuteChanged() const;

    rxcpp::observable<Track> trackMuteTouched() const;

    rxcpp::observable<Track> trackSoloChanged() const;

    rxcpp::observable<Track> trackSoloTouched() const;

    rxcpp::observable<Track> trackSelectedChanged() const;

    rxcpp::observable<Track> trackSelectedTouched() const;

    rxcpp::observable<Project> projectSwitched() const;

    rxcpp::observable<Track> trackInputChanged() const;

    rxcpp::observable<Track> trackPanTouched() const;

    rxcpp::observable<Track> trackArmTouched() const;

    rxcpp::observable<TrackSend> trackSendVolumeChanged() const;

    rxcpp::observable<TrackSend> trackSendVolumeTouched() const;

    rxcpp::observable<TrackSend> trackSendPanChanged() const;

    rxcpp::observable<TrackSend> trackSendPanTouched() const;

    rxcpp::observable<Track> trackAdded() const;

    rxcpp::observable<Track> trackRemoved() const;

    rxcpp::observable<Project> tracksReordered() const;

    rxcpp::observable<Fx> fxAdded() const;

    rxcpp::observable<Fx> fxRemoved() const;

    rxcpp::observable<Fx> fxEnabledChanged() const;

    rxcpp::observable<Fx> fxEnabledTouched() const;

    rxcpp::observable<Track> fxReordered() const;

    rxcpp::observable<bool> masterTempoChanged() const;

    rxcpp::observable<bool> masterTempoTouched() const;

    rxcpp::observable<bool> masterPlayrateChanged() const;

    rxcpp::observable<bool> masterPlayrateTouched() const;

    rxcpp::observable<bool> mainThreadIdle() const;

    rxcpp::composite_subscription enqueueCommand(std::function<void(void)> command);

    void enqueueCommandFast(std::function<void(void)> command);

    const rxcpp::observe_on_one_worker& mainThreadCoordination() const;

  private:
    HelperControlSurface();

    HelperControlSurface(const HelperControlSurface&);

    void removeInvalidReaProjects();

    void detectTrackSetChanges();

    void removeInvalidMediaTracks(const Project& project, TrackDataMap& trackDatas);

    void addMissingMediaTracks(const Project& project, TrackDataMap& trackDatas);

    void updateMediaTrackPositions(const Project& project, TrackDataMap& trackDatas);

    void detectFxChangesOnTrack(Track track, bool notifyListenersAboutChanges);

    // Returns true if FX was added or removed
    bool detectFxChangesOnTrack(Track track,
        std::set<std::string>& oldFxGuids,
        bool isInputFx,
        bool notifyListenersAboutChanges);

    void removeInvalidFx(Track track,
        std::set<std::string>& oldFxGuids,
        bool isInputFx,
        bool notifyListenersAboutChanges);

    void addMissingFx(Track track, std::set<std::string>& fxGuids, bool isInputFx, bool notifyListenersAboutChanges);

    std::set<std::string> fxGuidsOnTrack(Track track, bool isInputFx) const;

    bool isProbablyInputFx(Track track, int fxIndex, int paramIndex, double fxValue) const;

    bool isProbablyInputFx(Track track, int fxIndex) const;

    bool trackParameterIsAutomated(Track track, std::string parameterName) const;

    State state() const;

    TrackData* findTrackDataByTrack(MediaTrack* mediaTrack);
  };
}
