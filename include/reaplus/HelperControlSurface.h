#pragma once

#include <functional>
#include <unordered_map>
#include <set>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <mutex>
#include <boost/optional.hpp>
#include "reaper_plugin.h"
#include "rxcpp/rx.hpp"
#include "util/rx-relaxed-runloop.hpp"
#include "Project.h"
#include "Fx.h"
#include "FxParameter.h"
#include "Parameter.h"
#include "Track.h"
#include <concurrentqueue/concurrentqueue.h>

namespace reaplus {

  class FxParameter;

  class Track;

  class TrackSend;

  class Reaper;

  // DONE-rust
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

  // DONE-rust
  struct FxChainPair {
    std::set<std::string> inputFxGuids;
    std::set<std::string> outputFxGuids;
  };

  class HelperControlSurface : public IReaperControlSurface {
    friend class Reaper;
    friend class Track;
  private:
    // DONE-rust
    class Guard {
    public:
      ~Guard();
    };

    // DONE-rust
    enum class State {
      Normal,
      PropagatingTrackSetChanges
    };

    // DONE-rust
    static std::unique_ptr<HelperControlSurface> INSTANCE;
    // DONE-rust
    static constexpr int FAST_COMMAND_BUFFER_SIZE = 100;
    // DONE-rust
    int numTrackSetChangesLeftToBePropagated_ = 0;
    // DONE-rust
    rxcpp::subjects::subject<FxParameter> fxParameterValueChangedSubject_;
    // DONE-rust
    rxcpp::subjects::subject<FxParameter> fxParameterTouchedSubject_;
    // DONE-rust
    bool fxHasBeenTouchedJustAMomentAgo_ = false;
    // DONE-rust ...
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
    rxcpp::subjects::subject<Fx> fxOpenedSubject_;
    rxcpp::subjects::subject<Fx> fxClosedSubject_;
    rxcpp::subjects::subject<boost::optional<Fx>> fxFocusedSubject_;
    rxcpp::subjects::subject<Track> fxReorderedSubject_;
    rxcpp::subjects::subject<bool> masterTempoChangedSubject_;
    rxcpp::subjects::subject<bool> masterTempoTouchedSubject_;
    rxcpp::subjects::subject<bool> masterPlayrateChangedSubject_;
    rxcpp::subjects::subject<bool> masterPlayrateTouchedSubject_;
    rxcpp::subjects::subject<bool> mainThreadIdleSubject_;
    rxcpp::subjects::subject<Project> projectClosedSubject_;
    // DONE-rust
    rxcpp::subjects::behavior<Project> activeProjectBehavior_;
    // DONE-rust
    using TrackDataMap = std::unordered_map<MediaTrack*, TrackData>;
    // DONE-rust
    std::unordered_map<ReaProject*, TrackDataMap> trackDataByMediaTrackByReaProject_;
    // DONE-rust
    std::unordered_map<MediaTrack*, FxChainPair> fxChainPairByMediaTrack_;
    rxcpp::schedulers::relaxed_run_loop mainThreadRunLoop_;
    rxcpp::observe_on_one_worker mainThreadCoordination_ =
        rxcpp::observe_on_one_worker(rxcpp::schedulers::make_relaxed_run_loop(mainThreadRunLoop_));
    rxcpp::observe_on_one_worker::coordinator_type
        mainThreadCoordinator_ = mainThreadCoordination_.create_coordinator();
    // DONE-rust
    moodycamel::ConcurrentQueue<std::function<void(void)>> fastCommandQueue_;
    // DONE-rust
    std::array<std::function<void(void)>, FAST_COMMAND_BUFFER_SIZE> fastCommandBuffer_;

    // Capabilities depending on REAPER version
    // DONE-rust
    bool supportsDetectionOfInputFx_ = false;
    // DONE-rust
    bool supportsDetectionOfInputFxInSetFxChange_ = false;

  public:
    // DONE-rust
    ~HelperControlSurface() override;

    // DONE-rust
    void SetSurfacePan(MediaTrack* trackid, double pan) override;

    // DONE-rust
    void SetSurfaceVolume(MediaTrack* trackid, double volume) override;

    // DONE-rust
    const char* GetTypeString() override;

    // DONE-rust
    const char* GetDescString() override;

    // DONE-rust
    const char* GetConfigString() override;

    // DONE-rust
    void SetTrackListChange() override;

    // DONE-rust
    void Run() override;

    // DONE-rust
    int Extended(int call, void* parm1, void* parm2, void* parm3) override;

    // DONE-rust
    void SetTrackTitle(MediaTrack* trackid, const char* title) override;

    // DONE-rust
    void SetSurfaceMute(MediaTrack* trackid, bool mute) override;

    // DONE-rust
    void SetSurfaceSelected(MediaTrack* trackid, bool selected) override;

    // DONE-rust
    void SetSurfaceSolo(MediaTrack* trackid, bool solo) override;

    // DONE-rust
    void SetSurfaceRecArm(MediaTrack* trackid, bool recarm) override;

  protected:

    // DONE-rust
    static void init();

    // DONE-rust
    static HelperControlSurface& instance();

    // DONE-rust
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

    rxcpp::observable<Project> projectClosed() const;

    rxcpp::observable<Fx> fxAdded() const;

    rxcpp::observable<Fx> fxRemoved() const;

    rxcpp::observable<Fx> fxEnabledChanged() const;

    rxcpp::observable<Fx> fxEnabledTouched() const;

    rxcpp::observable<Track> fxReordered() const;

    rxcpp::observable<Fx> fxOpened() const;

    rxcpp::observable<Fx> fxClosed() const;

    rxcpp::observable<boost::optional<Fx>> fxFocused() const;

    rxcpp::observable<bool> masterTempoChanged() const;

    rxcpp::observable<bool> masterTempoTouched() const;

    rxcpp::observable<bool> masterPlayrateChanged() const;

    rxcpp::observable<bool> masterPlayrateTouched() const;

    rxcpp::observable<bool> mainThreadIdle() const;

    rxcpp::composite_subscription enqueueCommand(std::function<void(void)> command);

    void enqueueCommandFast(std::function<void(void)> command);

    const rxcpp::observe_on_one_worker& mainThreadCoordination() const;

  private:
    // DONE-rust
    HelperControlSurface();

    // DONE-rust
    HelperControlSurface(const HelperControlSurface&);

    // DONE-rust
    void removeInvalidReaProjects();

    // DONE-rust
    void detectTrackSetChanges();

    // DONE-rust
    void removeInvalidMediaTracks(const Project& project, TrackDataMap& trackDatas);

    // DONE-rust
    void addMissingMediaTracks(const Project& project, TrackDataMap& trackDatas);

    // DONE-rust
    void updateMediaTrackPositions(const Project& project, TrackDataMap& trackDatas);

    // DONE-rust
    void detectFxChangesOnTrack(Track track, bool notifyListenersAboutChanges,
        bool checkNormalFxChain, bool checkInputFxChain);

    // Returns true if FX was added or removed
    // DONE-rust
    bool detectFxChangesOnTrack(Track track,
        std::set<std::string>& oldFxGuids,
        bool isInputFx,
        bool notifyListenersAboutChanges);

    // DONE-rust
    void removeInvalidFx(Track track,
        std::set<std::string>& oldFxGuids,
        bool isInputFx,
        bool notifyListenersAboutChanges);

    // DONE-rust
    void addMissingFx(Track track, std::set<std::string>& fxGuids, bool isInputFx, bool notifyListenersAboutChanges);

    // DONE-rust
    std::set<std::string> fxGuidsOnTrack(Track track, bool isInputFx) const;

    // DONE-rust
    bool isProbablyInputFx(Track track, int fxIndex, int paramIndex, double fxValue) const;

    // DONE-rust
    bool trackParameterIsAutomated(Track track, std::string parameterName) const;

    // DONE-rust
    State state() const;

    // DONE-rust
    TrackData* findTrackDataByTrack(MediaTrack* mediaTrack);

    // DONE-rust
    // From REAPER > 5.95, parmFxIndex should be interpreted as query index. For earlier versions it's a normal index
    // - which unfortunately doesn't contain information if the FX is on the normal FX chain or the input FX chain.
    // In this case a heuristic is applied to determine which chain it is. It gets more accurate when paramIndex
    // and paramValue are supplied.
    boost::optional<Fx> getFxFromParmFxIndex(const Track& track, int parmFxIndex, int paramIndex = -1,
        int paramValue = -1) const;

    // DONE-rust
    void fxParamSet(void* parm1, void* parm2, void* parm3, bool isInputFxIfSupported);
  };
}
