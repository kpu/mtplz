#!/bin/bash
#Presumes cmake was run in the build directory
d="$(dirname "$0")"
"$d"/../build/bin/decode -p "$d"/test2.phrase_table.bin -K 3 -R 4 -l "$d"/test.arpa -W "$d"/test.weights <"$d"/test2.source_text --verbose
