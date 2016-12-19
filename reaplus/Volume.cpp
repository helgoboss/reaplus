#include "Volume.h"
#include <cmath>
#include "ModelUtil.h"
#include "utility.h"

#include "reaper_plugin_functions.h"

namespace reaplus {
  const double Volume::LN10_OVER_TWENTY = 0.11512925464970228420089957273422;

  Volume::Volume(double normalizedValue) : normalizedValue_(normalizedValue) {
  }
  
  Volume Volume::ofReaperValue(double reaperValue) {
    return Volume(reaper::DB2SLIDER(std::log(reaperValue) / LN10_OVER_TWENTY) / 1000.0);
  }
  
  double Volume::reaperValue() const {
    return std::exp(db() * LN10_OVER_TWENTY);
  }

  std::string Volume::toString() const {
    double db = this->db();
    if (db == -1000) {
      return std::string("-inf");
    } else {
      return reaplus::toString(20, [db](char* buffer, int maxSize) {
        std::sprintf(buffer, "%.2f dB", db);
      });
    }
  }

  double Volume::normalizedValue() const {
    return normalizedValue_;
  }

  double Volume::db() const {
    return reaper::SLIDER2DB(normalizedValue_ * 1000);
  }
}

