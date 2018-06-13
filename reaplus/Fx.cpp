#include "Fx.h"
#include "FxParameter.h"
#include "ModelUtil.h"
#include "FxChain.h"
#include <regex>

#include "reaper_plugin_functions.h"

#include "utility.h"

using rxcpp::subscriber;
using rxcpp::observable;
using std::function;
using std::string;
using std::unique_ptr;
using std::pair;
using std::string;

namespace reaplus {
  std::regex VST_LINE_REGEX("<VST \".*?\" ([^.]*).*");
  // FIXME What about JS effects with space inside?
  std::regex JS_LINE_REGEX("<JS ([^ ]+).*");

  int Fx::index() const {
    if (!isLoadedAndAtCorrectIndex()) {
      loadByGuid();
    }
    return index_;
  }

  string Fx::guid() const {
    return guid_;
  }

  string Fx::name() const {
    loadIfNecessaryOrComplain();
    return reaplus::toString(256, [this](char* buffer, int maxSize) {
      reaper::TrackFX_GetFXName(track_.mediaTrack(), queryIndex(), buffer, maxSize);
    });
  }

  Track Fx::track() const {
    return track_;
  }

  int Fx::queryIndex() const {
    loadIfNecessaryOrComplain();
    return Fx::queryIndex(index(), isInputFx());
  }

  bool Fx::isInputFx() const {
    return isInputFx_;
  }

  bool reaplus::operator==(const Fx& lhs, const Fx& rhs) {
    return lhs.track_ == rhs.track_ && lhs.isInputFx_ == rhs.isInputFx_ && lhs.guid_ == rhs.guid_;
  }

  Fx::Fx(Track track, string guid, bool isInputFx) : track_(track), guid_(guid), isInputFx_(isInputFx), index_(-1) {
  }

  string Fx::guid(Track track, int index, bool isInputFx) {
    const int queryIndex = Fx::queryIndex(index, isInputFx);
    const GUID* typeSafeGuid = reaper::TrackFX_GetFXGUID(track.mediaTrack(), queryIndex);
    if (typeSafeGuid == nullptr) {
      return "";
    } else {
      return convertGuidToString(*typeSafeGuid);
    }
  }

  int Fx::queryIndex(int index, bool isInputFx) {
    return (isInputFx ? 0x1000000 : 0) + index;
  }

  Fx::Fx(Track track, string guid, int index, bool isInputFx) : track_(track), guid_(guid), isInputFx_(isInputFx),
      index_(index) {
  }

  void Fx::invalidateIndex() const {
    loadByGuid();
  }

  void Fx::showInFloatingWindow() const {
    loadIfNecessaryOrComplain();
    reaper::TrackFX_Show(track().mediaTrack(), queryIndex(), 3);
  }

  bool Fx::windowIsOpen() const {
    return reaper::TrackFX_GetOpen(track().mediaTrack(), queryIndex());
  }

  pair<int, bool> Fx::indexFromQueryIndex(int queryIndex) {
    if (queryIndex >= 0x1000000) {
      return std::make_pair(queryIndex - 0x1000000, true);
    } else {
      return std::make_pair(queryIndex, false);
    }
  }

  HWND Fx::floatingWindow() const {
    loadIfNecessaryOrComplain();
    return reaper::TrackFX_GetFloatingWindow(track_.mediaTrack(), queryIndex());
  }

  bool Fx::moveForwardInPresetsBy(int count) {
    loadIfNecessaryOrComplain();
    return reaper::TrackFX_NavigatePresets(track_.mediaTrack(), queryIndex(), count);
  }

  bool Fx::moveBackwardInPresetsBy(int count) {
    loadIfNecessaryOrComplain();
    return reaper::TrackFX_NavigatePresets(track_.mediaTrack(), queryIndex(), -count);
  }

  bool Fx::presetIsDirty() const {
    loadIfNecessaryOrComplain();
    return !reaper::TrackFX_GetPreset(track_.mediaTrack(), queryIndex(), nullptr, 0);
  }

  std::string Fx::presetName() const {
    loadIfNecessaryOrComplain();
    return toString(2000, [this](char* buffer, int maxSize) {
      reaper::TrackFX_GetPreset(track_.mediaTrack(), queryIndex(), buffer, maxSize);
    });
  }

  int Fx::presetCount() const {
    loadIfNecessaryOrComplain();
    int presetCount;
    // TODO Integrate into ReaPlus
    reaper::TrackFX_GetPresetIndex(track_.mediaTrack(), queryIndex(), &presetCount);
    return presetCount;
  }

  bool reaplus::operator!=(const Fx& lhs, const Fx& rhs) {
    return !(lhs == rhs);
  }

  observable<FxParameter> Fx::parameters() const {
    loadIfNecessaryOrComplain();
    return observable<>::create<FxParameter>([this](subscriber<FxParameter> s) {
      const int count = parameterCount();
      for (int i = 0; i < count && s.is_subscribed(); i++) {
        s.on_next(parameterByIndex(i));
      }
      s.on_completed();
    });
  }

  int Fx::parameterCount() const {
    loadIfNecessaryOrComplain();
    return reaper::TrackFX_GetNumParams(track().mediaTrack(), queryIndex());
  }

  bool Fx::isAvailable() const {
    if (isLoadedAndAtCorrectIndex()) {
      // Loaded and at correct index
      return true;
    } else {
      // Not yet loaded or at wrong index
      return loadByGuid();
    }
  }

  FxChain Fx::chain() const {
    return isInputFx() ? track().inputFxChain() : track().normalFxChain();
  }

  bool Fx::loadByGuid() const {
    if (!chain().isAvailable()) {
      return false;
    }
    const auto foundFx = chain().fxs()
        .filter([this](Fx fx) {
          return fx.guid() == guid();
        })
        .map([](Fx fx) {
          return boost::make_optional(fx);
        })
        .default_if_empty((boost::optional<Fx>) boost::none)
        .as_blocking()
        .first();
    if (foundFx) {
      index_ = foundFx->index();
      return true;
    } else {
      return false;
    }
  }

  bool Fx::windowHasFocus() const {
    if (auto window = floatingWindow()) {
      // FX is open in floating window
      return GetActiveWindow() == window;
    } else {
      // FX is not open in floating window. In this case we consider it as focused if the FX chain of that track is
      // open and the currently displayed FX in the FX chain is this FX.
      return windowIsOpen();
    }
  }

  ChunkRegion Fx::chunk() const {
    loadIfNecessaryOrComplain();
    return chain().chunk()
        ->findLineStartingWith(fxIdLine())
        ->moveLeftCursorLeftToStartOfLineBeginningWith("BYPASS ")
        .moveRightCursorRightToStartOfLineBeginningWith("WAK 0")
        .moveRightCursorRightToEndOfCurrentLine();
  }

  bool Fx::isEnabled() const {
    return reaper::TrackFX_GetEnabled(track_.mediaTrack(), queryIndex());
  }

  void Fx::enable() {
    reaper::TrackFX_SetEnabled(track_.mediaTrack(), queryIndex(), true);
  }

  void Fx::disable() {
    reaper::TrackFX_SetEnabled(track_.mediaTrack(), queryIndex(), false);
  }

  string Fx::fileNameWithoutExtension() const {
    return Fx::fileNameWithoutExtension(tagChunk().firstLine());
  }

  string Fx::fileNameWithoutExtension(ChunkRegion firstLineOfTagChunk) {
    const auto regex = *firstLineOfTagChunk.tagName() == "VST" ? VST_LINE_REGEX : JS_LINE_REGEX;
    // FIXME Add type enumeration (JS, VST etc.)
    std::smatch match;
    const string firstLineOfTagChunkAsString = firstLineOfTagChunk.content().to_string();
    if (std::regex_match(firstLineOfTagChunkAsString, match, regex) && match.size() == 2) {
      return match[1].str();
    } else {
      return "";
    }
  }

  ChunkRegion Fx::tagChunk() const {
    loadIfNecessaryOrComplain();
    return chain().chunk()
        ->findLineStartingWith(fxIdLine())
        ->moveLeftCursorLeftToStartOfLineBeginningWith("BYPASS ")
        .findFirstTag(0)
        .get();
  }

  string Fx::fxIdLine() const {
    return Fx::fxIdLine(guid());
  }

  string Fx::fxIdLine(const string& guid) {
    return "FXID {" + guid + "}";
  }

  void Fx::setChunk(const char* chunk) {
    // First replace GUID in chunk with the one of this FX
    auto actualChunkString = std::make_shared<string>(chunk);
    auto actualChunk = Chunk(actualChunkString);
    if (auto fxIdLine = actualChunk.region().findLineStartingWith("FXID ")) {
      actualChunk.replaceRegion(*fxIdLine, Fx::fxIdLine(guid()));
    }
    // Then set new chunk
    replaceTrackChunkRegion(this->chunk(), actualChunk.content()->c_str());
  }

  void Fx::setTagChunk(const char* chunk) {
    replaceTrackChunkRegion(tagChunk(), chunk);
  }

  void Fx::replaceTrackChunkRegion(ChunkRegion oldChunkRegion, const char* newChunk) {
    oldChunkRegion.parentChunk().replaceRegion(oldChunkRegion, newChunk);
    track_.setChunk(oldChunkRegion.parentChunk().content()->c_str());
  }

  ChunkRegion Fx::stateChunk() const {
    return tagChunk().moveLeftCursorRightToStartOfNextLine().moveRightCursorLeftToEndOfPreviousLine();
  }

  void Fx::setStateChunk(const char* chunk) {
    replaceTrackChunkRegion(stateChunk(), chunk);
  }

  void Fx::loadIfNecessaryOrComplain() const {
    if (!isLoadedAndAtCorrectIndex() && !loadByGuid()) {
      throw std::logic_error("FX not loadable");
    }
  }

  FxParameter Fx::parameterByIndex(int index) const {
    return FxParameter(*this, index);
  }

  bool Fx::isLoadedAndAtCorrectIndex() const {
    if (index_ == -1) {
      // Not loaded
      return false;
    } else {
      // Loaded but might be at wrong index
      return track_.isAvailable() && guidByIndex() == guid_;
    }
  }

  string Fx::guidByIndex() const {
    return Fx::guid(track_, index_, isInputFx_);
  }
}
