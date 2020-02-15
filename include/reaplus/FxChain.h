#pragma once

#include <boost/optional.hpp>
#include <rxcpp/rx.hpp>
#include "Track.h"
#include "Chunk.h"

namespace reaplus {
  class Fx;
  class FxChain {
    friend class Track;
  private:
    // DONE-rust
    Track track_;
    bool isInputFx_;
  public:
    // DONE-rust
    int fxCount() const;
    // DONE-rust
    bool isInputFx() const;
    // DONE-rust
    rxcpp::observable<Fx> fxs() const;
    // It's correct that this returns an optional because the index isn't a stable identifier of an FX.
    // The FX could move. So this should do a runtime lookup of the FX and return a stable GUID-backed Fx object if
    // an FX exists at that index.
    // DONE-rust
    boost::optional<Fx> fxByIndex(int index) const;
    // DONE-rust
    boost::optional<Fx> firstInstrumentFx() const;
    // DONE-rust
    boost::optional<Fx> firstFx() const;
    // DONE-rust
    boost::optional<Fx> lastFx() const;
    // This returns a non-optional in order to support not-yet-loaded FX. GUID is a perfectly stable
    // identifier of an FX!
    // DONE-rust
    Fx fxByGuid(const std::string& guid) const;
    // Like fxByGuid but if you already know the index
    // DONE-rust
    Fx fxByGuidAndIndex(const std::string& guid, int index) const;
    // This returns a purely index-based FX that doesn't keep track of FX GUID, doesn't follow reorderings and so on.
    Fx fxByIndexUntracked(int index) const;
    // DONE-rust
    bool isAvailable() const;
    // DONE-rust
    boost::optional<Fx> addFxByOriginalName(const std::string& originalFxName);
    // TODO-rust
    boost::optional<Fx> addFxOfChunk(const char* chunk);
    // DONE-rust
    boost::optional<Fx> firstFxByName(const std::string& name) const;
    // DONE-rust
    boost::optional<ChunkRegion> chunk() const;
    // TODO-rust
    void setChunk(const char* chunk);
    // TODO-rust
    void removeFx(Fx fx);
    // TODO-rust
    void moveFx(Fx fx, int newIndex);
    // DONE-rust
    friend bool operator==(const FxChain& lhs, const FxChain& rhs);
    // DONE-rust
    friend bool operator!=(const FxChain& lhs, const FxChain& rhs);

  protected:
    // DONE-rust
    FxChain(Track track, bool isInputFx);

    // DONE-rust
    std::string chunkTagName() const;

    // DONE-rust
    boost::optional<ChunkRegion> findChunkRegion(Chunk trackChunk) const;
  };
}

