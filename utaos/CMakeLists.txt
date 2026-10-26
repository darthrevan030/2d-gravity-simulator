set(COMMON_SRC_FILES
  "${CMAKE_SOURCE_DIR}/aos/src/particle_system.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/integrate.cpp"
  "${CMAKE_SOURCE_DIR}/aos/src/collisions.cpp"
)

set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_collisions.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_integrate.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_particle_system.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_reference.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_equivalence.cpp"
)

add_unit_test_target(
  TARGET_NAME utaos
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER aos
  COVERAGE_DIR coverage-aos
  LIBRARY_TO_LINK common
  INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/aos/include" "${CMAKE_SOURCE_DIR}/testutil"
)
