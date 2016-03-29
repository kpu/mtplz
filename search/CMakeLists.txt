set (SEARCH_SOURCE
    edge_generator.cc
    nbest.cc
    rule.cc
    vertex.cc
)
add_library(mtplz_search ${SEARCH_SOURCE})
target_link_libraries(mtplz_search kenlm ${Boost_LIBRARIES})
