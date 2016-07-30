set(DECODE_SOURCE
  chart.cc
  coverage.cc
  distortion.cc
  filter.cc
  future.cc
  hypothesis_builder.cc
  lexro.cc
  lm.cc
  output.cc
  objective.cc
  system.cc
  score_collector.cc
  stacks.cc
  vocab_map.cc
  weights.cc)
add_library(mtplz_decode ${DECODE_SOURCE})
target_link_libraries(mtplz_decode mtplz_search mtplz_pt kenlm kenlm_util ${Boost_LIBRARIES})
target_compile_features(mtplz_decode PUBLIC cxx_range_for)

set(DECODE_LIBS mtplz_decode mtplz_search mtplz_pt kenlm kenlm_util ${Boost_LIBRARIES})

AddExes(EXES decode LIBRARIES ${DECODE_LIBS})

if(BUILD_TESTING)
  AddTests(TESTS coverage_test chart_test lexro_test LIBRARIES ${DECODE_LIBS})
endif()
