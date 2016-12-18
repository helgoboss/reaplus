#include "FxChain.h"
#include "Fx.h"
namespace reaper {
#include <reaper_plugin_functions.h>
}

using rxcpp::observable;
using rxcpp::subscriber;
using boost::optional;
using boost::none;
using std::string;

namespace reaplus {
  observable<Fx> FxChain::fxs() const {
    return observable<>::create<Fx>([this](subscriber<Fx> s) {
      const int fxCount = this->fxCount();
      for (int i = 0; i < fxCount && s.is_subscribed(); i++) {
        s.on_next(Fx(track_, Fx::guid(track_, i, isInputFx_), i, isInputFx_));
      }
      s.on_completed();
    });
  }

  optional<Fx> FxChain::fxByIndex(int index) const {
    if (index >= 0 && index < fxCount()) {
      return Fx(track_,  Fx::guid(track_, index, isInputFx_), index, isInputFx_);
    } else {
      return none;
    }
  }

  boost::optional<Fx> FxChain::firstInstrumentFx() const {
    if (isInputFx_) {
      return none;
    } else {
      const int fxIndex = reaper::TrackFX_GetInstrument(track_.mediaTrack());
      return fxByIndex(fxIndex);
    }
  }

  int FxChain::fxCount() const {
    return isInputFx_ ? reaper::TrackFX_GetRecCount(track_.mediaTrack()) : reaper::TrackFX_GetCount(track_.mediaTrack());
  }

  FxChain::FxChain(Track track, bool isInputFx) : track_(track), isInputFx_(isInputFx) {

  }

  Fx FxChain::fxByGuid(const string& guid) const {
    return Fx(track_, guid, isInputFx_);
  }

  Fx FxChain::fxByGuidAndIndex(const string& guid, int index) const {
    return Fx(track_, guid, index, isInputFx_);
  }

  bool FxChain::isAvailable() const {
    return track_.isAvailable();
  }

  boost::optional<Fx> FxChain::addFxByOriginalName(const string& originalFxName) {
    const int fxIndex = reaper::TrackFX_AddByName(track_.mediaTrack(), originalFxName.c_str(), isInputFx_, -1);
    if (fxIndex == -1) {
      return boost::none;
    } else {
      return Fx(track_,  Fx::guid(track_, fxIndex, isInputFx_), fxIndex, isInputFx_);
    }
  }

  boost::optional<Fx> FxChain::firstFxByName(const string& name) const {
    const int fxIndex = reaper::TrackFX_AddByName(track_.mediaTrack(), name.c_str(), isInputFx_, 0);
    if (fxIndex == -1) {
      return boost::none;
    } else {
      return Fx(track_,  Fx::guid(track_, fxIndex, isInputFx_), fxIndex, isInputFx_);
    }
  }

  bool reaplus::operator==(const FxChain& lhs, const FxChain& rhs) {
    return lhs.track_ == rhs.track_ && lhs.isInputFx_ == rhs.isInputFx_;
  }

  bool reaplus::operator!=(const FxChain& lhs, const FxChain& rhs) {
    return !(lhs == rhs);
  }


  boost::optional<ChunkRegion> FxChain::chunk() const {
    return findChunkRegion(track_.chunk());
  }

  string FxChain::chunkTagName() const {
    return isInputFx_ ? "FXCHAIN_REC" : "FXCHAIN";
  }

  boost::optional<Fx> FxChain::addFxOfChunk(const char* chunk) {
    auto trackChunk = track_.chunk();
    auto chainTag = findChunkRegion(trackChunk);
    if (chainTag) {
      // There's an FX chain already. Add after last FX.
      trackChunk.insertBeforeRegionAsBlock(chainTag->lastLine(), chunk);
    } else {
      // There's no FX chain yet. Insert it with FX.
      string chainChunkString = R"foo(
<FXCHAIN
WNDRECT 0 144 1082 736
SHOW 0
LASTSEL 1
DOCKED 0
)foo";
      chainChunkString.append(chunk);
      chainChunkString.append("\n>");
      trackChunk.insertAfterRegionAsBlock(trackChunk.region().firstLine(), chainChunkString);
    }
    track_.setChunk(trackChunk.content()->c_str());
    return lastFx();
  }

  void FxChain::moveFx(Fx fx, int newIndex) {
    if (fx.isAvailable() && fx.index() != newIndex) {
      const int actualNewIndex = std::min(newIndex, fxCount() - 1);
      const auto originalFxChunkRegion = fx.chunk();
      const string fxChunkString = originalFxChunkRegion.content().to_string();
      const auto currentFxAtNewIndexChunkRegion = fxByIndex(actualNewIndex)->chunk();
      if (fx.index() < actualNewIndex) {
        // Moves down
        originalFxChunkRegion.parentChunk().insertAfterRegionAsBlock(currentFxAtNewIndexChunkRegion, fxChunkString);
        originalFxChunkRegion.parentChunk().deleteRegion(originalFxChunkRegion);
      } else {
        // Moves up
        originalFxChunkRegion.parentChunk().deleteRegion(originalFxChunkRegion);
        originalFxChunkRegion.parentChunk().insertBeforeRegionAsBlock(currentFxAtNewIndexChunkRegion, fxChunkString);
      }
      track_.setChunk(originalFxChunkRegion.parentChunk().content()->c_str());
    }
  }

  void FxChain::setChunk(const char* chunk) {
    auto trackChunk = track_.chunk();
    auto chainTag = findChunkRegion(trackChunk);
    if (chainTag) {
      // There's an FX chain already. Replace it.
      trackChunk.replaceRegion(*chainTag, chunk);
    } else {
      // There's no FX chain yet. Insert it.
      trackChunk.insertAfterRegionAsBlock(trackChunk.region().firstLine(), chunk);
    }
    track_.setChunk(trackChunk.content()->c_str());
  }

  bool FxChain::isInputFx() const {
    return isInputFx_;
  }

  boost::optional<Fx> FxChain::firstFx() const {
    return fxByIndex(0);
  }

  boost::optional<Fx> FxChain::lastFx() const {
    const auto fxCount = this->fxCount();
    if (fxCount == 0) {
      return none;
    } else {
      return fxByIndex(fxCount - 1);
    }
  }

  void FxChain::removeFx(Fx fx) {
    if (fx.isAvailable()) {
      const auto fxChunkRegion = fx.chunk();
      fxChunkRegion.parentChunk().deleteRegion(fxChunkRegion);
      track_.setChunk(fxChunkRegion.parentChunk().content()->c_str());
    }
  }

  optional<ChunkRegion> FxChain::findChunkRegion(Chunk trackChunk) const {
    return trackChunk.region().findFirstTagNamed(0, chunkTagName());
  }

}