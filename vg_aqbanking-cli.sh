#!/bin/bash

#export AQHBCI_LOGLEVEL=info 
export LD_LIBRARY_PATH="build/src/libs/:$LD_LIBRARY_PATH"


valgrind \
  --tool=memcheck --trace-children=yes -v --log-file=aqbanking-cli.vg --leak-check=full --show-reachable=yes \
  --track-origins=yes --num-callers=50 --keep-stacktraces=alloc-and-free \
  build/src/tools/aqbanking-cli/aqbanking-cli "$@"

