#!/bin/bash
if [ ${#} != 2 ]; then
  echo "Usage: $0 phrase_table.gz reordering_table.gz >phrase_table.binary" 1>&2
  exit 1
fi
BIN="$(dirname "$0")/../build/bin"
paste -d \| <("$BIN"/cat_compressed "$1" |cut -d \| -f 1-9) <("$BIN"/cat_compressed "$1" |cut -d \| -f 7-) |"$BIN"/binarize_phrase_table -c source target dense_features lexical_reordering
