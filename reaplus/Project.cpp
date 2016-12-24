#include "Project.h"
#include "Track.h"
#include "Section.h"
#include "Action.h"
#include "utility.h"
#include <reaper_plugin_functions.h>

using rxcpp::subscriber;
using boost::none;
using std::string;
using std::function;
using std::unique_ptr;
using rxcpp::observable;
using boost::optional;


namespace reaplus {
  Project::Project(ReaProject* reaProject): reaProject_(reaProject) {
  }

  observable<Track> Project::tracks() const {
    complainIfNotAvailable();
    return observable<>::create<Track>([this](subscriber<Track> s) {
      const int trackCount = this->trackCount();
      for (int i = 0; i < trackCount && s.is_subscribed(); i++) {
        auto mediaTrack = reaper::GetTrack(reaProject_, i);
        s.on_next(Track(mediaTrack, reaProject()));
      }
      s.on_completed();
    });
  }

  ReaProject* Project::reaProject() const {
    return reaProject_;
  }

  Track Project::trackByGuid(const string& guid) const {
    complainIfNotAvailable();
    return Track(reaProject_, guid);
  }

  int Project::index() const {
    complainIfNotAvailable();
    for (int i = 0; true; i++) {
      auto reaProject = reaper::EnumProjects(i, nullptr, 0);
      if (reaProject == nullptr) {
        return -1;
      } else if (reaProject == reaProject_) {
        return i;
      }
    }
  }

  boost::optional<std::string> Project::filePath() const {
    auto p = toString(5000, [this](char* buffer, int maxSize) {
      reaper::EnumProjects(index(), buffer, maxSize);
    });
    if (p.empty()) {
      return boost::none;
    } else {
      return p;
    }
  }

  bool reaplus::operator==(const Project& lhs, const Project& rhs) {
    return lhs.reaProject_ == rhs.reaProject_;
  }

  bool reaplus::operator!=(const Project& lhs, const Project& rhs) {
    return !(lhs.reaProject_ == rhs.reaProject_);
  }

  optional<Track> Project::trackByIndex(int index) const {
    if (index < 0) {
      return none;
    } else {
      complainIfNotAvailable();
      auto mediaTrack = reaper::GetTrack(reaProject_, index);
      if (mediaTrack == nullptr) {
        return none;
      } else {
        return Track(mediaTrack, reaProject());
      }
    }
  }

  boost::optional<Track> Project::trackByNumber(int number) const {
    if (number == 0) {
      return masterTrack();
    } else {
      return trackByIndex(number - 1);
    }
  }

  Track Project::masterTrack() const {
    complainIfNotAvailable();
    return Track(reaper::GetMasterTrack(reaProject_), reaProject());
  }

  Track Project::addTrack() {
    complainIfNotAvailable();
    return insertTrackAt(trackCount());
  }

  Track Project::insertTrackAt(int index) {
    complainIfNotAvailable();
    // TODO reaper::InsertTrackAtIndex unfortunately doesn't allow to specify ReaProject :(
    reaper::InsertTrackAtIndex(index, false);
    auto mediaTrack = reaper::GetTrack(reaProject_, index);
    return Track(mediaTrack, reaProject());
  }

  rxcpp::observable<Track> Project::selectedTracks() const {
    complainIfNotAvailable();
    return observable<>::create<Track>([this](subscriber<Track> s) {
      const int count = selectedTrackCount();
      for (int i = 0; i < count && s.is_subscribed(); i++) {
        auto mediaTrack = reaper::GetSelectedTrack2(reaProject_, i, false);
        s.on_next(Track(mediaTrack, reaProject()));
      }
      s.on_completed();
    });
  }

  boost::optional<Track> Project::firstSelectedTrack() const {
    const auto mediaTrack = reaper::GetSelectedTrack2(reaProject_, 0, false);
    if (mediaTrack) {
      return Track(mediaTrack, reaProject_);
    } else {
      return none;
    }
  }

  int Project::selectedTrackCount() const {
    return reaper::CountSelectedTracks2(reaProject_, false);
  }

  void Project::removeTrack(Track track) {
    reaper::DeleteTrack(track.mediaTrack());
  }

  optional<Track> Project::firstTrack() const {
    return trackByIndex(0);
  }

  void Project::complainIfNotAvailable() const {
    if (!isAvailable()) {
      throw std::logic_error("Project not available");
    }
  }

  optional<Track> Project::lastTrack() const {
    const auto trackCount = this->trackCount();
    if (trackCount == 0) {
      return none;
    } else {
      return trackByIndex(trackCount - 1);
    }
  }

  int Project::trackCount() const {
    complainIfNotAvailable();
    return reaper::CountTracks(reaProject());
  }

  bool Project::isAvailable() const {
    return reaper::ValidatePtr2(nullptr, reaProject_, "ReaProject*");
  }


}

