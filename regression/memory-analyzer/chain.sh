#!/bin/bash

set -e

goto_gcc="../../../build/bin/goto-gcc"
memory_analyzer=$1
name=${*:$#}
name=${name%.exe}
args=${*:2:$#-2}

$goto_gcc -g -std=c11 -o "${name}.exe" "${name}.c"
$memory_analyzer $args "${name}.exe"
