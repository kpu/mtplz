set (PT_SOURCE
  access.cc
  create.cc
  format.cc
  query.cc
)
add_library(mtplz_pt ${PT_SOURCE})
target_link_libraries(mtplz_pt kenlm_util)
target_compile_features(mtplz_pt PUBLIC cxx_range_for)
