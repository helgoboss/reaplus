#pragma once

#include <functional>
#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <mutex>
#include "reaper_plugin.h"
#include "rxcpp/rx.hpp"
#include "Project.h"
#include "Fx.h"
#include "FxParameter.h"
#include "Parameter.h"
#include "Track.h"

namespace reaplus {

  class FxParameter;

  class Track;

  class TrackSend;

  class Reaper;

  struct FxChainPair {
    std::set<std::string> inputFxGuids;
    std::set<std::string> outputFxGuids;
  };

  class HelperControlSurface : public IReaperControlSurface {
    friend class Reaper;

  private:
    rxcpp::subjects::subject<FxParameter> fxParameterValueChangedSubject_;
    rxcpp::subjects::subject<FxParameter> fxParameterTouchedSubject_;
    bool fxHasBeenTouchedJustAMomentAgo_ = false;
    rxcpp::subjects::subject<Track> trackVolumeChangedSubject_;
    rxcpp::subjects::subject<Track> trackVolumeTouchedSubject_;
    rxcpp::subjects::subject<Track> trackPanChangedSubject_;
    rxcpp::subjects::subject<Track> trackPanTouchedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendVolumeChangedSubject_;
    rxcpp::subjects::subject<TrackSend> trackSendVolumeTouchedSubject_;
    rxcpp::subjects::subject<Track> trackAddedSubject_;
    rxcpp::subjects::subject<Track> trackRemovedSubject_;
    rxcpp::subjects::subject<Project> tracksReorderedSubject_;
    rxcpp::subjects::subject<Fx> fxAddedSubject_;
    rxcpp::subjects::subject<Fx> fxRemovedSubject_;
    rxcpp::subjects::subject<Track> fxReorderedSubject_;
    std::unordered_map<ReaProject*, std::set<MediaTrack*>> mediaTracksByReaProject_;
    std::unordered_map<MediaTrack*, FxChainPair> fxChainPairByMediaTrack_;
    rxcpp::schedulers::run_loop mainThreadRunLoop_;
    rxcpp::observe_on_one_worker mainThreadCoordination_ = rxcpp::observe_on_run_loop(mainThreadRunLoop_);
    rxcpp::observe_on_one_worker::coordinator_type mainThreadCoordinator_ = mainThreadCoordination_.create_coordinator();

  public:

    virtual void SetSurfacePan(MediaTrack* trackid, double pan) override;

    virtual void SetSurfaceVolume(MediaTrack* trackid, double volume) override;

    const char* GetTypeString();

    const char* GetDescString();

    const char* GetConfigString();

    virtual void SetTrackListChange() override;

    void Run();

    virtual int Extended(int call, void* parm1, void* parm2, void* parm3) override;

  protected:

    static void init();

    static HelperControlSurface& instance();

    rxcpp::observable<Parameter*> parameterValueChangedUnsafe() const;

    rxcpp::observable<Parameter*> parameterTouchedUnsafe() const;

    rxcpp::observable<FxParameter> fxParameterValueChanged() const;

    rxcpp::observable<FxParameter> fxParameterTouched() const;

    rxcpp::observable<Track> trackVolumeChanged() const;

    rxcpp::observable<Track> trackVolumeTouched() const;

    rxcpp::observable<Track> trackPanChanged() const;

    rxcpp::observable<Track> trackPanTouched() const;

    rxcpp::observable<TrackSend> trackSendVolumeChanged() const;

    rxcpp::observable<TrackSend> trackSendVolumeTouched() const;

    rxcpp::observable<Track> trackAdded() const;

    rxcpp::observable<Track> trackRemoved() const;

    rxcpp::observable<Project> tracksReordered() const;

    rxcpp::observable<Fx> fxAdded() const;

    rxcpp::observable<Fx> fxRemoved() const;

    rxcpp::observable<Track> fxReordered() const;

    rxcpp::subscription enqueueCommand(std::function<void(void)> command);

    const rxcpp::observe_on_one_worker& mainThreadCoordination() const;

  private:
    HelperControlSurface();
    // TODO Delete copy method

    void removeInvalidReaProjects();

    void detectTrackSetChanges();

    void removeInvalidMediaTracks(const Project& project, std::set<MediaTrack*>& mediaTracks);

    void addMissingMediaTracks(const Project& project, std::set<MediaTrack*>& mediaTracks);

    void detectFxChangesOnTrack(Track track);

    // Returns true if FX was added or removed
    bool detectFxChangesOnTrack(Track track, std::set<std::string>& oldFxGuids, bool isInputFx);

    void removeInvalidFx(Track track, std::set<std::string>& oldFxGuids, bool isInputFx);

    void addMissingFx(Track track, std::set<std::string>& fxGuids, bool isInputFx);

    std::set<std::string> fxGuidsOnTrack(Track track, bool isInputFx) const;

    bool isProbablyInputFx(Track track, int fxIndex, int paramIndex, double fxValue) const;

    bool isProbablyInputFx(Track track, int fxIndex) const;

    bool trackParameterIsAutomated(Track track, std::string parameterName) const;
  };
}
