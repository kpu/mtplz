#!/bin/bash
#Presumes cmake was run in the build directory
d="$(dirname "$0")"
"$d"/../build/bin/decode -p "$d"/test.phrase_table -K 1 -R 1 -l "$d"/test.arpa -W "$d"/test.weights <"$d"/test.source_text
