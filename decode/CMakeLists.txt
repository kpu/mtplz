set(DECODE_SOURCE
  chart.cc
  coverage.cc
  filter.cc
  future.cc
  output.cc
  phrase.cc
  phrase_table.cc
  scorer.cc
  objective.cc
  stacks.cc
  weights.cc)
add_library(mtplz_decode ${DECODE_SOURCE})
target_link_libraries(mtplz_decode mtplz_search kenlm kenlm_util ${Boost_LIBRARIES})

set(DECODE_LIBS mtplz_decode mtplz_search kenlm kenlm_util ${Boost_LIBRARIES})

AddExes(EXES decode LIBRARIES ${DECODE_LIBS})

if(BUILD_TESTING)
  AddTests(TESTS coverage_test LIBRARIES ${DECODE_LIBS})
endif()
