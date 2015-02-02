{-# LANGUAGE ForeignFunctionInterface, CPP #-}
module Main where

import Foreign
import Foreign.Ptr
import Foreign.C.String
import Foreign.C.Types
import HTime

main = getLine >>= profileCommand >>= (return.show) >>= putStrLn
