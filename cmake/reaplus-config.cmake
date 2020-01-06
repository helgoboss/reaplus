include(CMakeFindDependencyMacro)
# TODO
find_dependency(helgoboss-midi 0.1.0 CONFIG REQUIRED)
find_dependency(unofficial-concurrentqueue CONFIG REQUIRED)
find_dependency(rxcpp CONFIG REQUIRED)
find_dependency(Boost REQUIRED COMPONENTS filesystem exception)
find_dependency(spdlog CONFIG REQUIRED)
# TODO Remove dependency to helgoboss-learn
find_dependency(helgoboss-learn 0.1.0 CONFIG REQUIRED)
include("${CMAKE_CURRENT_LIST_DIR}/reaplus-targets.cmake")