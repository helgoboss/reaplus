#pragma once

#include <string>
#include "Track.h"
#include <functional>
#include <memory>
#include <boost/optional.hpp>
#include "FxChain.h"
#include "Chunk.h"

namespace reaplus {

  class FxParameter;
  class Fx {
    friend class FxChain;
    friend class Track;
  private:
    // TODO Save chain instead of track
    Track track_;
    // Primary identifier
    std::string guid_;
    // Secondary identifier, can become invalid on FX reorderings
    // TODO Use boost::none instead of -1
    mutable int index_;
    bool isInputFx_;
  public:
    // To be called if you become aware that this FX might have been affected by a reordering.
    // Note that the Fx also corrects the index itself whenever one of its methods is called.
    void invalidateIndex() const;
    bool isAvailable() const;
    int index() const;
    int queryIndex() const;
    std::string guid() const;
    std::string name() const;
    ChunkRegion chunk() const;
    void setChunk(const char* chunk);
    ChunkRegion tagChunk() const;
    void setTagChunk(const char* chunk);
    ChunkRegion stateChunk() const;
    void setStateChunk(const char* chunk);
    std::string fileNameWithoutExtension() const;
    Track track() const;
    bool isInputFx() const;
    FxChain chain() const;
    int parameterCount() const;
    bool isEnabled() const;
    void enable();
    void disable();
    // Non-optional because we consider the index as a stable
    // ID (which is not completely true because the plugin can add and remove parameters ... but we anyway don't have
    // a way to represent a parameter in a stable way).
    FxParameter parameterByIndex(int index) const;
    rxcpp::observable<FxParameter> parameters() const;
    void showInFloatingWindow() const;
    HWND floatingWindow() const;
    bool windowIsOpen() const;
    bool windowHasFocus() const;
    bool moveForwardInPresetsBy(int count);
    bool moveBackwardInPresetsBy(int count);
    bool presetIsDirty() const;
    std::string presetName() const;
    int presetCount() const;
    friend bool operator==(const Fx& lhs, const Fx& rhs);
    friend bool operator!=(const Fx& lhs, const Fx& rhs);
  protected:
    // Main constructor. Use it if you have the GUID. index will be determined lazily.
    Fx(Track track, std::string guid, bool isInputFx);
    // Use this constructor if you are sure about the GUID and index
    Fx(Track track, std::string guid, int index, bool isInputFx);
    static int queryIndex(int index, bool isInputFx);
    static std::pair<int, bool> indexFromQueryIndex(int queryIndex);
    // Returns empty string if no FX at that index
    static std::string guid(Track track, int index, bool isInputFx);
    static std::string fileNameWithoutExtension(ChunkRegion firstLineOfTagChunk);
  private:
    static std::string fxIdLine(const std::string& guid);
    bool loadByGuid() const;

    void loadIfNecessaryOrComplain() const;

    bool isLoadedAndAtCorrectIndex() const;

    // Returns empty string if no FX at that index anymore
    // TODO Use boost::optional
    std::string guidByIndex() const;

    std::string fxIdLine() const;

    void replaceTrackChunkRegion(ChunkRegion oldChunkRegion, const char* newChunk);
  };
}

