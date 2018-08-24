#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "reaplus/ModelUtil.h"

using namespace reaplus;

// -1   -1    1 => 0
// -0.5 -1    1 => 0.25
// 0    -1    1 => 0.5
// 0.5  -1    1 => 0.75
// 1    -1    1 => 1
// -1   0     1 => -1
// -0.5 0     1 => -0.5
// 0    0     1 => 0
// 0.5  0     1 => 0.5
// 1    0     1 => 1
// -2   1     2 => -1
// -1.5 1     2 => -0.5
// -1   1     2 => 0
// 1    1     2 => 0
// 1.5  1     2 => 0.5
// 2    1     2 => 1

TEST_CASE("ModelUtil 1") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-1, -1, 1) == 0);
}

TEST_CASE("ModelUtil 2") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-0.5, -1, 1) == 0.25);
}
TEST_CASE("ModelUtil 3") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(0, -1, 1) == 0.5);
}
TEST_CASE("ModelUtil 4") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(0.5, -1, 1) == 0.75);
}
TEST_CASE("ModelUtil 5") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(1, -1, 1) == 1);
}





TEST_CASE("ModelUtil 6") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-1, 0, 1) == -1);
}
TEST_CASE("ModelUtil 7") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-0.5, 0, 1) == -0.5);
}
TEST_CASE("ModelUtil 8") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(0, 0, 1) == 0);
}
TEST_CASE("ModelUtil 9") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(0.5, 0, 1) == 0.5);
}
TEST_CASE("ModelUtil 10") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(1, 0, 1) == 1);
}




TEST_CASE("ModelUtil 11") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-2, 1, 2) == -1);
}
TEST_CASE("ModelUtil 12") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-1.5, 1, 2) == -0.5);
}
TEST_CASE("ModelUtil 13") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(-1, 1, 2) == 0);
}
TEST_CASE("ModelUtil 14") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(1, 1, 2) == 0);
}
TEST_CASE("ModelUtil 15") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(1.5, 1, 2) == 0.5);
}
TEST_CASE("ModelUtil 16") {
  REQUIRE(ModelUtil::mapValueInRangeToNormalizedValue(2, 1, 2) == 1);
}