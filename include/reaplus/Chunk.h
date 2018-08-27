#pragma once

#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>

namespace reaplus {
  class ChunkRegion;

  class Chunk {
  public:
    explicit Chunk(std::shared_ptr<std::string> content);

    std::shared_ptr<std::string> content() const;

    ChunkRegion region() const;

    void insertBeforeRegion(ChunkRegion region, const char* chunk);

    void insertBeforeRegionAsBlock(ChunkRegion region, const char* chunk);

    void insertBeforeRegion(ChunkRegion region, const std::string& chunk);

    void insertBeforeRegionAsBlock(ChunkRegion region, const std::string& chunk);

    void insertAfterRegion(ChunkRegion region, const char* chunk);

    void insertAfterRegionAsBlock(ChunkRegion region, const char* chunk);

    void insertAfterRegion(ChunkRegion region, const std::string& chunk);

    void insertAfterRegionAsBlock(ChunkRegion region, const std::string& chunk);

    void encloseRegion(const char* prefix, ChunkRegion region, const char* suffix);

    void encloseRegion(const std::string& prefix, ChunkRegion region, const std::string& suffix);

    void replaceRegion(ChunkRegion region, const char* chunk);

    void replaceRegion(ChunkRegion region, const std::string& chunk);

    void deleteRegion(ChunkRegion region);

  private:
    std::shared_ptr<std::string> content_;

    void requireValidRegion(ChunkRegion region) const;

    void insertNewLinesIfNecessaryAt(size_t pos1, size_t pos2);

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
    Chunk parentChunk() const;

    size_t startPos() const;

    size_t length() const;

    size_t endPosPlusOne() const;

    boost::string_ref content() const;

    bool startsWith(const std::string& needle) const;

    bool endsWith(const std::string& needle) const;

    bool contains(const std::string& needle) const;

    ChunkRegion firstLine() const;

    ChunkRegion lastLine() const;

    boost::optional<std::string> tagName() const;

    bool isTag() const;

    boost::optional<ChunkRegion> findFirstString(const std::string& needle) const;

    boost::optional<ChunkRegion> findFirstStringAtLineStart(const std::string& needle) const;

    boost::optional<ChunkRegion> findLineStartingWith(const std::string& needle) const;

    // Returns the tag completely from < to >
    boost::optional<ChunkRegion> findFirstTagNamed(size_t relativeSearchStartPos, const std::string& tagName) const;

    // Returns the tag completely from < to >
    // TODO Why don't we return an invalid chunk region instead of none? That would allow easier chaining and would
    // be more in line with the other methods.
    boost::optional<ChunkRegion> findFirstTag(size_t relativeSearchStartPos) const;

    ChunkRegion moveLeftCursorLeftToStartOf(const std::string& needle) const;

    ChunkRegion moveLeftCursorLeftToStartOfLineBeginningWith(const std::string& needle) const;

    ChunkRegion moveLeftCursorRightBy(size_t count) const;

    ChunkRegion moveLeftCursorLeftBy(size_t count) const;

    ChunkRegion moveLeftCursorRightToStartOf(const std::string& needle) const;

    ChunkRegion moveLeftCursorRightToStartOfNextLine() const;

    ChunkRegion moveLeftCursorLeftToStartOfCurrentLine() const;

    ChunkRegion moveLeftCursorRightToStartOfLineBeginningWith(const std::string& needle) const;

    ChunkRegion moveRightCursorLeftToStartOf(const std::string& needle) const;

    ChunkRegion moveRightCursorLeftToStartOfLineBeginningWith(const std::string& needle) const;

    ChunkRegion moveRightCursorRightBy(size_t count) const;

    ChunkRegion moveRightCursorLeftBy(size_t count) const;

    ChunkRegion moveLeftCursorToRightCursor() const;

    ChunkRegion moveRightCursorToLeftCursor() const;

    ChunkRegion moveRightCursorRightToStartOf(const std::string& needle) const;

    ChunkRegion moveRightCursorRightToEndOfCurrentLine() const;

    ChunkRegion moveRightCursorLeftToEndOfPreviousLine() const;

    ChunkRegion moveRightCursorRightToStartOfLineBeginningWith(const std::string& needle) const;

    ChunkRegion moveLeftCursorLeftToStartOfPreviousLine() const;

    ChunkRegion moveRightCursorRightToEndOfNextLine() const;

    bool isValid() const;

    ChunkRegion before() const;

    ChunkRegion after() const;

  private:
    ChunkRegion(Chunk parentChunk, size_t startPos, size_t length);

    size_t findFollowedByOneOf(const std::string& needle, const std::string& oneOf, size_t relStartPos) const;

    boost::optional<ChunkRegion> parseTagStartingFrom(size_t relTagOpenerPos) const;

    ChunkRegion createRegionFromRelativeStartPos(size_t relStartPos, size_t length) const;

    ChunkRegion moveLeftCursorTo(size_t startPos) const;

    ChunkRegion createInvalidRegion() const;
  };

}