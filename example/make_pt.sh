build/bin/binarize_phrase_table \
    -c source target dense_features lexical_reordering \
    < example/test.phrase_table \
    > example/test.phrase_table.bin
build/bin/binarize_phrase_table \
    -c source target dense_features lexical_reordering \
    < example/test2.phrase_table \
    > example/test2.phrase_table.bin
