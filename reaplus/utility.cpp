#include "utility.h"
#include <algorithm>
#include <cctype>

#include "reaper_plugin_functions.h"

using std::string;
using std::function;

namespace reaplus {
  string convertGuidToString(const GUID& guid) {
    string guidString = toString(64, [&guid](char* buffer, int maxSize) {
      reaper::guidToString((GUID*) &guid, buffer);
    });
    // Erase braces
    guidString.erase(guidString.length() - 1);
    guidString.erase(0, 1);
    return guidString;
  }

  string toString(int maxSize, function<void(char*, int)> fillBuffer) {
    // TODO Can this be implemented in a better way?
    string s;
    s.resize((size_t) maxSize);
    fillBuffer(&s[0], maxSize);
    s.resize(s.find('\0'));
    return s;
  }

  std::shared_ptr<string> toSharedString(int maxSize, function<void(char*, int)> fillBuffer) {
    auto s = std::make_shared<string>();
    s->resize((size_t) maxSize);
    fillBuffer(&(*s)[0], maxSize);
    s->resize(s->find('\0'));
    return s;
  }

}