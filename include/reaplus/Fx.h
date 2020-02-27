#pragma once

#include <string>
#include "Track.h"
#include <functional>
#include <memory>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include "FxChain.h"
#include "Chunk.h"

namespace reaplus {

  // DONE-rust
  class FxInfo {
  private:
    // DONE-rust
    std::string effectName_;
    std::string typeExpression_;
    std::string subTypeExpression_;
    std::string vendorName_;
    boost::filesystem::path fileName_;
  public:
    // DONE-rust
    explicit FxInfo(const std::string& firstLineOfTagChunk);
    // e.g. ReaSynth, currently empty if JS
    // DONE-rust
    std::string getEffectName() const;
    // e.g. VST or JS
    // DONE-rust
    std::string getTypeExpression() const;
    // e.g. VSTi, currently empty if JS
    // DONE-rust
    std::string getSubTypeExpression() const;
    // e.g. Cockos, currently empty if JS
    // DONE-rust
    std::string getVendorName() const;
    // e.g. reasynth.dll or phaser
    // DONE-rust
    boost::filesystem::path getFileName() const;
  };

  class FxParameter;
  class Fx {
    friend class FxChain;
    friend class Track;
  private:
    // TODO Save chain instead of track
    // DONE-rust
    Track track_;
    // Primary identifier, but only for tracked, GUID-based FX instances. Otherwise empty.
    // DONE-rust
    std::string guid_;
    // For GUID-based FX instances this is the secondary identifier, can become invalid on FX reorderings.
    // For just index-based FX instances this is the primary identifier.
    // TODO Use boost::none instead of -1
    // DONE-rust
    mutable int index_;
    // DONE-rust
    bool isInputFx_;
  public:
    // To be called if you become aware that this FX might have been affected by a reordering.
    // Note that the Fx also corrects the index itself whenever one of its methods is called.
    // TODO-rust
    void invalidateIndex() const;
    // DONE-rust
    bool isAvailable() const;
    // DONE-rust
    int index() const;
    // DONE-rust
    int queryIndex() const;
    // DONE-rust
    std::string guid() const;
    // DONE-rust
    std::string name() const;
    // Attention: Currently implemented by parsing chunk
    // DONE-rust
    FxInfo getFxInfo() const;
    // DONE-rust
    ChunkRegion chunk() const;
    // TODO-rust
    void setChunk(const char* chunk);
    // DONE-rust
    ChunkRegion tagChunk() const;
    // TODO-rust
    void setTagChunk(const char* chunk);
    // DONE-rust
    ChunkRegion stateChunk() const;
    // TODO-rust
    void setStateChunk(const char* chunk);
    Track track() const;
    // DONE-rust
    bool isInputFx() const;
    // DONE-rust
    FxChain chain() const;
    // DONE-rust
    int parameterCount() const;
    // DONE-rust
    bool isEnabled() const;
    // DONE-rust
    void enable();
    // DONE-rust
    void disable();
    // Non-optional because we consider the index as a stable
    // ID (which is not completely true because the plugin can add and remove parameters ... but we anyway don't have
    // a way to represent a parameter in a stable way).
    // DONE-rust
    FxParameter parameterByIndex(int index) const;
    // DONE-rust
    rxcpp::observable<FxParameter> parameters() const;
    // TODO-rust
    void showInFloatingWindow() const;
    // TODO-rust
    HWND floatingWindow() const;
    // TODO-rust
    bool windowIsOpen() const;
    // TODO-rust
    bool windowHasFocus() const;
    // TODO-rust
    bool moveForwardInPresetsBy(int count);
    // TODO-rust
    bool moveBackwardInPresetsBy(int count);
    // TODO-rust
    bool presetIsDirty() const;
    // TODO-rust
    std::string presetName() const;
    // TODO-rust
    int presetCount() const;
    // TODO-rust
    int presetIndex() const;
    // TODO-rust
    void loadPreset(int presetIndex);
    // DONE-rust
    friend bool operator==(const Fx& lhs, const Fx& rhs);
    // DONE-rust
    friend bool operator!=(const Fx& lhs, const Fx& rhs);
  protected:
    // Main constructor. Use it if you have the GUID. index will be determined lazily.
    // DONE-rust
    Fx(Track track, std::string guid, bool isInputFx);
    // Use this constructor if you are sure about the GUID and index
    // DONE-rust
    Fx(Track track, std::string guid, int index, bool isInputFx);
    // Use this if you want to create a purely index-based FX without UUID tracking
    Fx(Track track, int index, bool isInputFx);
    // DONE-rust
    static int queryIndex(int index, bool isInputFx);
    // DONE-rust
    static std::pair<int, bool> indexFromQueryIndex(int queryIndex);
    // Returns empty string if no FX at that index
    // DONE-rust
    static std::string guid(Track track, int index, bool isInputFx);
  private:
    // DONE-rust
    static std::string fxIdLine(const std::string& guid);
    // DONE-rust
    bool loadByGuid() const;

    // DONE-rust
    void loadIfNecessaryOrComplain() const;

    // DONE-rust
    bool isLoadedAndAtCorrectIndex() const;

    // Returns empty string if no FX at that index anymore
    // TODO Use boost::optional
    // DONE-rust
    std::string guidByIndex() const;

    // DONE-rust
    std::string fxIdLine() const;

    void replaceTrackChunkRegion(ChunkRegion oldChunkRegion, const char* newChunk);
  };
}

