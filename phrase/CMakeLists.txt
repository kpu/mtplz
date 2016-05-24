set (PT_SOURCE
  access.cc
  format.cc
)
add_library(mtplz_phrase ${PT_SOURCE})
target_link_libraries(mtplz_phrase kenlm_util)
target_compile_features(mtplz_phrase PUBLIC cxx_range_for)
