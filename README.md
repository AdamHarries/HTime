# HTime
This library provides functionality similar to the GNU `time` command within plain haskell code. This is implemented using FFI bindings to a stripped down library-like version of the `time` utility. Timing code derived from that at http://ftp.heanet.ie/mirrors/gnu/time/

Note: this software is provided along the same terms of the original license of the `time` utility, however adding explicit license information to this repo is still at the "TODO" stage.

TODO:
 - Test, test, test!
 - Clean up & modernise C code
 - Add better functions for extracting timing data
 - Implement JSON serialisation for timing structures
 - Document Haskell interface/datastructures
