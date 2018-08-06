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
    Track track_;
    bool isInputFx_;
  public:
    int fxCount() const;
    bool isInputFx() const;
    rxcpp::observable<Fx> fxs() const;
    // It's correct that this returns an optional because the index isn't a stable identifier of an FX.
    // The FX could move. So this should do a runtime lookup of the FX and return a stable GUID-backed Fx object if
    // an FX exists at that index.
    boost::optional<Fx> fxByIndex(int index) const;
    boost::optional<Fx> firstInstrumentFx() const;
    boost::optional<Fx> firstFx() const;
    boost::optional<Fx> lastFx() const;
    // This returns a non-optional in order to support not-yet-loaded FX. GUID is a perfectly stable
    // identifier of an FX!
    Fx fxByGuid(const std::string& guid) const;
    // Like fxByGuid but if you already know the index
    Fx fxByGuidAndIndex(const std::string& guid, int index) const;
    // This returns a purely index-based FX that doesn't keep track of FX GUID, doesn't follow reorderings and so on.
    Fx fxByIndexUntracked(int index) const;
    bool isAvailable() const;
    boost::optional<Fx> addFxByOriginalName(const std::string& originalFxName);
    boost::optional<Fx> addFxOfChunk(const char* chunk);
    boost::optional<Fx> firstFxByName(const std::string& name) const;
    boost::optional<ChunkRegion> chunk() const;
    void setChunk(const char* chunk);
    void removeFx(Fx fx);
    void moveFx(Fx fx, int newIndex);
    friend bool operator==(const FxChain& lhs, const FxChain& rhs);
    friend bool operator!=(const FxChain& lhs, const FxChain& rhs);

  protected:
    FxChain(Track track, bool isInputFx);

    std::string chunkTagName() const;

    boost::optional<ChunkRegion> findChunkRegion(Chunk trackChunk) const;
  };
}

