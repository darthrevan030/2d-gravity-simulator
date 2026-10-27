set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/common/src/progargs.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/input_parser.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/config_writer.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_progargs.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_point.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_input_parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config_writer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_physics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_collision_rules.cpp"
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)