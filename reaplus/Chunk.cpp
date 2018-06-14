#include "Chunk.h"
#include <cmath>
#include <cstring>

using boost::none;
using boost::string_ref;
using boost::optional;
using std::pair;
using std::string;

namespace reaplus {
  std::shared_ptr<string> Chunk::content() const {
    return content_;
  }

  Chunk::Chunk(std::shared_ptr<string> content) : content_(content) {
  }

  ChunkRegion Chunk::region() const {
    return ChunkRegion(*this, 0, content_->size());
  }

  string_ref ChunkRegion::content() const {
    const auto fullStringRef = string_ref(*parentChunk_.content());
    if (isValid()) {
      return fullStringRef.substr(startPos_, length_);
    } else {
      return fullStringRef.substr(0, 0);
    }
  }

  optional<string> ChunkRegion::tagName() const {
    if (isTag()) {
      const auto contentWithoutLowerThan = content().substr(1);
      const auto endPosPlusOne = contentWithoutLowerThan.find_first_of(" \n");
      if (endPosPlusOne == string::npos) {
        return none;
      } else {
        return contentWithoutLowerThan.substr(0, endPosPlusOne).to_string();
      }
    } else {
      return none;
    }
  }

  optional<ChunkRegion> ChunkRegion::findFirstTagNamed(size_t relativeSearchStartPos, const string& tagName) const {
    if (isValid()) {
      const string tagOpenerWithNewLine = string("\n<") + tagName;
      const size_t tagOpenerWithNewLinePos = findFollowedByOneOf(
          tagOpenerWithNewLine,
          string(" \n"),
          relativeSearchStartPos
      );
      if (tagOpenerWithNewLinePos == string::npos) {
        return none;
      } else {
        return parseTagStartingFrom(tagOpenerWithNewLinePos + 1);
      }
    } else {
      return none;
    }
  }

  optional<ChunkRegion> ChunkRegion::findFirstTag(size_t relativeSearchStartPos) const {
    if (isValid()) {
      if (content().substr(relativeSearchStartPos).starts_with("<")) {
        return parseTagStartingFrom(relativeSearchStartPos);
      } else {
        const string tagOpenerWithNewLine = string("\n<");
        const size_t superRelativeTagOpenerWithNewLinePos =
            content().substr(relativeSearchStartPos).find(tagOpenerWithNewLine);
        if (superRelativeTagOpenerWithNewLinePos == string::npos) {
          return none;
        } else {
          const size_t relTagOpenerWithNewLinePos = relativeSearchStartPos + superRelativeTagOpenerWithNewLinePos;
          return parseTagStartingFrom(relTagOpenerWithNewLinePos + 1);
        }
      }
    } else {
      return none;
    }
  }

  optional<ChunkRegion> ChunkRegion::parseTagStartingFrom(size_t relTagOpenerPos) const {
    // Precondition: isValid
    if (relTagOpenerPos == string::npos) {
      // Tag opener not found
      return none;
    } else {
      // Tag opener found
      size_t relStartPos = relTagOpenerPos + 1;
      int openLevelsCount = 1;
      while (relStartPos < content().length()) {
        const size_t relTagOpenerOrCloserPos = findFollowedByOneOf(string("\n"), string("<>"), relStartPos);
        if (relTagOpenerOrCloserPos == string::npos) {
          // No further tag opener or closer found
          return none;
        } else {
          // Further tag opener or closer found
          const size_t relTagOpenerOrCloserWithoutNewlinePos = relTagOpenerOrCloserPos + 1;
          const char tagOpenerOrCloserWithoutNewline = content()[relTagOpenerOrCloserWithoutNewlinePos];
          if (tagOpenerOrCloserWithoutNewline == '<') {
            // Opening tag (nested)
            openLevelsCount++;
            relStartPos = relTagOpenerOrCloserWithoutNewlinePos + 1;
          } else {
            // Closing tag
            openLevelsCount--;
            if (openLevelsCount == 0) {
              // Found tag closer of searched tag
              const size_t length = relTagOpenerOrCloserWithoutNewlinePos - relTagOpenerPos + 1;
              return createRegionFromRelativeStartPos(relTagOpenerPos, length);
            } else {
              // Nested tag was closed
              relStartPos = relTagOpenerOrCloserWithoutNewlinePos + 1;
            }
          }
        }
      }
      // Tag closer not found
      return none;
    }
  }

  ChunkRegion ChunkRegion::createRegionFromRelativeStartPos(size_t relStartPos, size_t length) const {
    return ChunkRegion(parentChunk_, startPos_ + relStartPos, length);
  }

  size_t ChunkRegion::endPosPlusOne() const {
    return startPos() + length();
  }

  size_t ChunkRegion::findFollowedByOneOf(const string& needle, const string& oneOf, size_t relStartPos) const {
    // Precondition: isValid
    while (relStartPos < content().length()) {
      size_t needlePosRelativeToRelStartPos = content().substr(relStartPos).find(needle);
      if (needlePosRelativeToRelStartPos == string::npos) {
        // Needle not found
        return string::npos;
      } else {
        // Needle found
        size_t relNeedlePos = relStartPos + needlePosRelativeToRelStartPos;
        size_t relFollowingCharPos = relNeedlePos + needle.length();
        if (relFollowingCharPos < content().length()) {
          // String goes on after needle
          char followingChar = content()[relFollowingCharPos];
          if (oneOf.find(followingChar) != string::npos) {
            // Found complete match
            return relNeedlePos;
          } else {
            // No complete match yet. Go on searching.
            relStartPos = relFollowingCharPos + 1;
          }
        } else {
          // No complete match found
          return string::npos;
        }
      }
    }
    // No complete match found
    return string::npos;
  }

  void Chunk::replaceRegion(ChunkRegion region, const char* chunk) {
    requireValidRegion(region);
    content_->replace(region.startPos(), region.length(), chunk);
  }

  void Chunk::requireValidRegion(ChunkRegion region) const {
    if (!region.isValid()) {
      throw std::logic_error("Invalid region");
    }
  }

  optional<ChunkRegion> ChunkRegion::findFirstStringAtLineStart(const string& needle) const {
    if (isValid()) {
      if (content().size() < needle.size()) {
        return none;
      } else if (content().substr(0, needle.size()) == needle) {
        return ChunkRegion(parentChunk_, 0, needle.length());
      } else {
        const auto relPos = content().find(string("\n") + needle);
        if (relPos == string::npos) {
          return none;
        } else {
          return ChunkRegion(parentChunk_, startPos() + relPos + 1, needle.size());
        }
      }
    } else {
      return none;
    }
  }

  optional<ChunkRegion> ChunkRegion::findLineStartingWith(const string& needle) const {
    if (isValid()) {
      const auto needleRegion = findFirstStringAtLineStart(needle);
      if (needleRegion) {
        return needleRegion->moveRightCursorRightToEndOfCurrentLine();
      } else {
        return none;
      }
    } else {
      return none;
    }
  }

  void Chunk::insertBeforeRegion(ChunkRegion region, const string& chunk) {
    insertBeforeRegion(region, chunk.c_str());
  }

  void Chunk::insertBeforeRegionAsBlock(ChunkRegion region, const string& chunk) {
    insertBeforeRegionAsBlock(region, chunk.c_str());
  }

  void Chunk::insertBeforeRegionAsBlock(ChunkRegion region, const char* chunk) {
    insertBeforeRegion(region, chunk);
    insertNewLinesIfNecessaryAt(region.startPos(), region.startPos() + std::strlen(chunk));
  }

  void Chunk::insertAfterRegionAsBlock(ChunkRegion region, const char* chunk) {
    insertAfterRegion(region, chunk);
    insertNewLinesIfNecessaryAt(region.endPosPlusOne(), region.endPosPlusOne() + std::strlen(chunk));
  }

  void Chunk::insertAfterRegionAsBlock(ChunkRegion region, const string& chunk) {
    insertAfterRegionAsBlock(region, chunk.c_str());
  }

  void Chunk::insertAfterRegion(ChunkRegion region, const string& chunk) {
    insertAfterRegion(region, chunk.c_str());
  }

  void Chunk::encloseRegion(const string& prefix, ChunkRegion region, const string& suffix) {
    encloseRegion(prefix.c_str(), region, suffix.c_str());
  }

  void Chunk::replaceRegion(ChunkRegion region, const string& chunk) {
    replaceRegion(region, chunk.c_str());
  }

  void Chunk::deleteRegion(ChunkRegion region) {
    requireValidRegion(region);
    content_->erase(region.startPos(), region.length());
  }

  ChunkRegion::ChunkRegion(Chunk parentChunk, size_t startPos, size_t length) : parentChunk_(parentChunk),
      startPos_(startPos), length_(length) {
  }

  size_t ChunkRegion::startPos() const {
    return startPos_;
  }

  size_t ChunkRegion::length() const {
    return length_;
  }

  ChunkRegion ChunkRegion::firstLine() const {
    if (isValid()) {
      const size_t relPos = content().find_first_of('\n');
      if (relPos == string::npos) {
        return *this;
      } else {
        return createRegionFromRelativeStartPos(0, relPos);
      }
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::lastLine() const {
    if (isValid()) {
      const size_t relPosOfNewline = content().find_last_of('\n');
      if (relPosOfNewline == string::npos) {
        return *this;
      } else {
        const size_t relPosAfterNewline = relPosOfNewline + 1;
        return createRegionFromRelativeStartPos(relPosAfterNewline, length() - relPosAfterNewline);
      }
    } else {
      return *this;
    }
  }

  bool ChunkRegion::contains(const string& needle) const {
    if (isValid()) {
      return content().find(needle) != string::npos;
    } else {
      return false;
    }
  }

  void Chunk::insertBeforeRegion(ChunkRegion region, const char* chunk) {
    requireValidRegion(region);
    content_->insert(region.startPos(), chunk);
  }

  void Chunk::insertAfterRegion(ChunkRegion region, const char* chunk) {
    requireValidRegion(region);
    content_->insert(region.endPosPlusOne(), chunk);
  }

  void Chunk::encloseRegion(const char* prefix, ChunkRegion region, const char* suffix) {
    requireValidRegion(region);
    insertBeforeRegion(region, prefix);
    content_->insert(region.endPosPlusOne() + std::strlen(prefix), suffix);
  }

  void Chunk::insertNewLinesIfNecessaryAt(size_t pos1, size_t pos2) {
    const bool inserted = insertNewLineIfNecessaryAt(pos1);
    insertNewLineIfNecessaryAt(inserted ? pos2 + 1 : pos2);
  }

  bool Chunk::insertNewLineIfNecessaryAt(size_t pos) {
    if (pos == 0) {
      return false;
    } else if (pos >= content_->length() - 1) {
      return false;
    } else {
      if (content_->at(pos) != '\n') {
        content_->insert(pos, "\n");
        return true;
      } else {
        return false;
      }
    }
  }

  bool ChunkRegion::isTag() const {
    if (isValid()) {
      return !content().empty() && content()[0] == '<';
    } else {
      return false;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorLeftToStartOf(const string& needle) const {
    if (isValid()) {
      const auto before = this->before();
      if (before.isValid()) {
        const size_t startPosOfNeedle = before.content().rfind(needle);
        if (startPosOfNeedle == string::npos) {
          return createInvalidRegion();
        } else {
          return moveLeftCursorTo(startPosOfNeedle);
        }
      } else {
        return createInvalidRegion();
      }
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorRightBy(size_t count) const {
    if (isValid()) {
      return moveLeftCursorTo(startPos() + count);
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorRightToStartOf(const string& needle) const {
    if (isValid()) {
      const size_t relPos = content().find(needle);
      if (relPos == string::npos) {
        return createInvalidRegion();
      } else {
        return moveLeftCursorRightBy(relPos);
      }
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorLeftToStartOfLineBeginningWith(const string& needle) const {
    return moveLeftCursorLeftToStartOf("\n" + needle).moveLeftCursorRightBy(1);
  }

  ChunkRegion ChunkRegion::moveLeftCursorRightToStartOfLineBeginningWith(const string& needle) const {
    return moveLeftCursorRightToStartOf("\n" + needle).moveLeftCursorRightBy(1);
  }

  ChunkRegion ChunkRegion::moveRightCursorLeftToStartOfLineBeginningWith(const string& needle) const {
    return moveRightCursorLeftToStartOf("\n" + needle).moveRightCursorRightBy(1);
  }

  ChunkRegion ChunkRegion::moveRightCursorRightToStartOfLineBeginningWith(const string& needle) const {
    return moveRightCursorRightToStartOf("\n" + needle).moveRightCursorRightBy(1);
  }

  ChunkRegion ChunkRegion::moveRightCursorLeftToStartOf(const string& needle) const {
    if (isValid()) {
      const size_t relStartPosOfNeedle = content().rfind(needle);
      if (relStartPosOfNeedle == string::npos) {
        return createInvalidRegion();
      } else {
        return createRegionFromRelativeStartPos(0, relStartPosOfNeedle);
      }
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorLeftBy(size_t count) const {
    if (isValid()) {
      return moveLeftCursorTo(startPos() - count);
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveRightCursorLeftBy(size_t count) const {
    if (isValid()) {
      return createRegionFromRelativeStartPos(0, length() - count);
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::before() const {
    if (isValid()) {
      return ChunkRegion(parentChunk_, 0, startPos());
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::after() const {
    if (isValid()) {
      return ChunkRegion(parentChunk_, endPosPlusOne(), parentChunk_.content()->size() - endPosPlusOne());
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveRightCursorRightBy(size_t count) const {
    if (isValid()) {
      return createRegionFromRelativeStartPos(0, length() + count);
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveRightCursorRightToStartOf(const string& needle) const {
    if (isValid()) {
      const auto after = this->after();
      if (after.isValid()) {
        const size_t relPos = after.content().find(needle);
        if (relPos == string::npos) {
          return createInvalidRegion();
        } else {
          return moveRightCursorRightBy(relPos);
        }
      } else {
        return *this;
      }
    } else {
      return *this;
    }
  }

  ChunkRegion ChunkRegion::moveRightCursorRightToEndOfNextLine() const {
    return moveRightCursorRightToStartOfLineBeginningWith("").moveRightCursorRightToEndOfCurrentLine();
  }

  ChunkRegion ChunkRegion::moveLeftCursorLeftToStartOfPreviousLine() const {
    return moveLeftCursorLeftToStartOf("\n").moveLeftCursorLeftToStartOfCurrentLine();
  }

  ChunkRegion ChunkRegion::moveLeftCursorTo(size_t startPos) const {
    if (isValid()) {
      const size_t newLength = length() + (startPos_ - startPos);
      return ChunkRegion(parentChunk_, startPos, newLength);
    } else {
      return *this;
    }
  }

  bool ChunkRegion::isValid() const {
    return length_ > 0 && startPos_ + length_ <= parentChunk_.content()->length();
  }

  optional<ChunkRegion> ChunkRegion::findFirstString(const string& needle) const {
    if (isValid()) {
      const auto relPos = content().find(needle);
      if (relPos == string::npos) {
        return none;
      } else {
        return ChunkRegion(parentChunk_, startPos() + relPos, needle.size());
      }
    } else {
      return none;
    }
  }

  ChunkRegion ChunkRegion::moveLeftCursorRightToStartOfNextLine() const {
    return moveLeftCursorRightToStartOfLineBeginningWith("");
  }

  ChunkRegion ChunkRegion::moveLeftCursorLeftToStartOfCurrentLine() const {
    return moveLeftCursorLeftToStartOfLineBeginningWith("");
  }

  ChunkRegion ChunkRegion::moveRightCursorRightToEndOfCurrentLine() const {
    return moveRightCursorRightToStartOf("\n");
  }

  ChunkRegion ChunkRegion::moveRightCursorLeftToEndOfPreviousLine() const {
    return moveRightCursorLeftToStartOf("\n");
  }

  ChunkRegion ChunkRegion::createInvalidRegion() const {
    return ChunkRegion(parentChunk_, startPos_, 0);
  }

  bool ChunkRegion::startsWith(const string& needle) const {
    if (isValid()) {
      return content().substr(0, needle.length()) == needle;
    } else {
      return false;
    }
  }

  bool ChunkRegion::endsWith(const string& needle) const {
    if (isValid()) {
      return content().substr(content().length() - needle.length()) == needle;
    } else {
      return false;
    }
  }

  Chunk ChunkRegion::parentChunk() const {
    return parentChunk_;
  }
}