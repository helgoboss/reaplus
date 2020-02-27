#pragma once

#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace reaplus {
  class ChunkRegion;

  // TODO-rust
  class Chunk {
  public:
    // DONE-rust
    explicit Chunk(std::shared_ptr<std::string> content);

    // DONE-rust
    std::shared_ptr<std::string> content() const;

    // DONE-rust
    ChunkRegion region() const;

    void insertBeforeRegion(ChunkRegion region, const char* chunk);

    void insertBeforeRegionAsBlock(ChunkRegion region, const char* chunk);

    void insertBeforeRegion(ChunkRegion region, const std::string& chunk);

    void insertBeforeRegionAsBlock(ChunkRegion region, const std::string& chunk);

    void insertAfterRegion(ChunkRegion region, const char* chunk);

    // DONE-rust
    void insertAfterRegionAsBlock(ChunkRegion region, const char* chunk);

    void insertAfterRegion(ChunkRegion region, const std::string& chunk);

    void insertAfterRegionAsBlock(ChunkRegion region, const std::string& chunk);

    void encloseRegion(const char* prefix, ChunkRegion region, const char* suffix);

    void encloseRegion(const std::string& prefix, ChunkRegion region, const std::string& suffix);

    void replaceRegion(ChunkRegion region, const char* chunk);

    void replaceRegion(ChunkRegion region, const std::string& chunk);

    // DONE-rust
    void deleteRegion(ChunkRegion region);

  private:
    // DONE-rust
    std::shared_ptr<std::string> content_;

    // DONE-rust
    void requireValidRegion(ChunkRegion region) const;

    // DONE-rust
    void insertNewLinesIfNecessaryAt(size_t pos1, size_t pos2);

    // DONE-rust
    bool insertNewLineIfNecessaryAt(size_t pos);
  };

  class ChunkRegion {
    friend class Chunk;

  private:
    Chunk parentChunk_;
    // startPos_ = 0 and length_ = 0 means it's invalid
    size_t startPos_;
    size_t length_;

  public:
    // DONE-rust
    Chunk parentChunk() const;

    // DONE-rust
    size_t startPos() const;

    // DONE-rust
    size_t length() const;

    // DONE-rust
    size_t endPosPlusOne() const;

    // DONE-rust
    boost::string_ref content() const;

    // DONE-rust
    bool startsWith(const std::string& needle) const;

    // DONE-rust
    bool endsWith(const std::string& needle) const;

    // DONE-rust
    bool contains(const std::string& needle) const;

    // DONE-rust
    ChunkRegion firstLine() const;

    ChunkRegion lastLine() const;

    boost::optional<std::string> tagName() const;

    bool isTag() const;

    boost::optional<ChunkRegion> findFirstString(const std::string& needle) const;

    // DONE-rust
    boost::optional<ChunkRegion> findFirstStringAtLineStart(const std::string& needle) const;

    // DONE-rust
    boost::optional<ChunkRegion> findLineStartingWith(const std::string& needle) const;

    // Returns the tag completely from < to >
    // DONE-rust
    boost::optional<ChunkRegion> findFirstTagNamed(size_t relativeSearchStartPos, const std::string& tagName) const;

    // Returns the tag completely from < to >
    // TODO Why don't we return an invalid chunk region instead of none? That would allow easier chaining and would
    // be more in line with the other methods.
    // DONE-rust
    boost::optional<ChunkRegion> findFirstTag(size_t relativeSearchStartPos) const;

    // DONE-rust
    ChunkRegion moveLeftCursorLeftToStartOf(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveLeftCursorLeftToStartOfLineBeginningWith(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveLeftCursorRightBy(size_t count) const;

    ChunkRegion moveLeftCursorLeftBy(size_t count) const;

    // DONE-rust
    ChunkRegion moveLeftCursorRightToStartOf(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveLeftCursorRightToStartOfNextLine() const;

    ChunkRegion moveLeftCursorLeftToStartOfCurrentLine() const;

    // DONE-rust
    ChunkRegion moveLeftCursorRightToStartOfLineBeginningWith(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveRightCursorLeftToStartOf(const std::string& needle) const;

    ChunkRegion moveRightCursorLeftToStartOfLineBeginningWith(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveRightCursorRightBy(size_t count) const;

    ChunkRegion moveRightCursorLeftBy(size_t count) const;

    ChunkRegion moveLeftCursorToRightCursor() const;

    ChunkRegion moveRightCursorToLeftCursor() const;

    // DONE-rust
    ChunkRegion moveRightCursorRightToStartOf(const std::string& needle) const;

    // DONE-rust
    ChunkRegion moveRightCursorRightToEndOfCurrentLine() const;

    // DONE-rust
    ChunkRegion moveRightCursorLeftToEndOfPreviousLine() const;

    // DONE-rust
    ChunkRegion moveRightCursorRightToStartOfLineBeginningWith(const std::string& needle) const;

    ChunkRegion moveLeftCursorLeftToStartOfPreviousLine() const;

    ChunkRegion moveRightCursorRightToEndOfNextLine() const;

    // DONE-rust
    bool isValid() const;

    // DONE-rust
    ChunkRegion before() const;

    // DONE-rust
    ChunkRegion after() const;

  private:
    // DONE-rust
    ChunkRegion(Chunk parentChunk, size_t startPos, size_t length);

    // DONE-rust
    size_t findFollowedByOneOf(const std::string& needle, const std::string& oneOf, size_t relStartPos) const;

    // DONE-rust
    boost::optional<ChunkRegion> parseTagStartingFrom(size_t relTagOpenerPos) const;

    // DONE-rust
    ChunkRegion createRegionFromRelativeStartPos(size_t relStartPos, size_t length) const;

    // DONE-rust
    ChunkRegion moveLeftCursorTo(size_t startPos) const;

    // DONE-rust
    ChunkRegion createInvalidRegion() const;
  };

}