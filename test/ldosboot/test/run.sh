#! /bin/bash

# Usage of the works is permitted provided that this
# instrument is retained with the works, so that any entity
# that uses the works is notified of this instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

testrunname="fat12 default: " \
 ./test.sh diskette
testrunname="fat12 direct: " \
 ./test.sh diskette direct
testrunname="fat12 hdimage: " \
 ./test.sh hdimage
testrunname="fat12 16spc 20MiB: " \
 ./test.sh hdimage aligndata bpe 12 spc 16 mib 20 nr 512
testrunname="fat16 4spc 32MiB: " \
 ./test.sh hdimage aligndata bpe 16 spc 4 mib 32 nr 512
testrunname="fat32 1spc 34MiB: " \
 ./test.sh hdimage aligndata bpe 32 spc 1 mib 34
