{-# LANGUAGE ForeignFunctionInterface #-}
 
module HTime (profileCommand, resuseElapsed) where

import Foreign
import Foreign.Ptr
import Foreign.C.String
import Foreign.C.Types
import Control.Concurrent
import System.Posix.Signals


#include "resuse.h"

foreign import ccall unsafe "run_command" cRunCommand :: CString -> Ptr Resuse -> IO CInt

profileCommand :: String -> IO Resuse
profileCommand cmd = alloca $ \resPtr -> do
	rc <- withCString cmd (\s -> cRunCommand s resPtr)
	if rc == 0
		then peek resPtr
		else error ("got error " ++ (show rc) ++ " from command call")


--foreign import ccall unsafe "profileCommand" 
--	cProfileCommand :: CString -> IO Resuse

--timeCommand :: String -> IO Timeval
--timeCommand cmd = withCString cmd cProfileCommand >>= (return.resuseElapsed)

resuseElapsed :: Resuse -> Timeval
resuseElapsed (Resuse ws ru st et) = et


data Timeval = Timeval {
	seconds :: Integer,
	microseconds :: Integer 
} deriving (Show)

instance Storable Timeval where
	sizeOf		_ = (#size struct timeval)
	alignment	_ = alignment (undefined :: CDouble)
	peek ptr = do 
		sec 	<- (#peek struct timeval, tv_sec) ptr :: IO CInt
		usec 	<- (#peek struct timeval, tv_usec) ptr :: IO CInt
		return Timeval {seconds = (toInteger sec), microseconds = (toInteger usec)}
	poke ptr (Timeval sec usec) = do
		(#poke struct timeval, tv_sec) ptr (fromIntegral sec :: CInt)
		(#poke struct timeval, tv_usec) ptr (fromIntegral usec :: CInt)

data RUsage = RUsage {
	utime :: Timeval,
	stime :: Timeval,
	maxrss :: Int,
	ixrss :: Int,
	idrss :: Int,
	isrss :: Int,
	minflt :: Int,
	majflt :: Int,
	nswap :: Int,
	inblock :: Int,
	oublock :: Int,
	msgsnd :: Int,
	msgrcv :: Int,
	nsignals :: Int,
	nvcsw :: Int,
	nivcsw :: Int
} deriving (Show)

instance Storable RUsage where
	sizeOf		_ = (#size struct rusage)
	alignment	_ = alignment (undefined :: CDouble)
	peek ptr = do
		r_utime		<- (#peek struct rusage, ru_utime) ptr
		r_stime		<- (#peek struct rusage, ru_stime) ptr
		r_maxrss	<- (#peek struct rusage, ru_maxrss) ptr
		r_ixrss		<- (#peek struct rusage, ru_ixrss) ptr
		r_idrss		<- (#peek struct rusage, ru_idrss) ptr
		r_isrss		<- (#peek struct rusage, ru_isrss) ptr
		r_minflt	<- (#peek struct rusage, ru_minflt) ptr
		r_majflt	<- (#peek struct rusage, ru_majflt) ptr
		r_nswap		<- (#peek struct rusage, ru_nswap) ptr
		r_inblock	<- (#peek struct rusage, ru_inblock) ptr
		r_oublock	<- (#peek struct rusage, ru_oublock) ptr
		r_msgsnd	<- (#peek struct rusage, ru_msgsnd) ptr
		r_msgrcv	<- (#peek struct rusage, ru_msgrcv) ptr
		r_nsignals	<- (#peek struct rusage, ru_nsignals) ptr
		r_nvcsw		<- (#peek struct rusage, ru_nvcsw) ptr
		r_nivcsw	<- (#peek struct rusage, ru_nivcsw) ptr
		return RUsage {
			utime = r_utime,
			stime = r_stime,
			maxrss = r_maxrss,
			ixrss = r_ixrss,
			idrss = r_idrss,
			isrss = r_isrss,
			minflt = r_minflt,
			majflt = r_majflt,
			nswap = r_nswap,
			inblock = r_inblock,
			oublock = r_oublock,
			msgsnd = r_msgsnd,
			msgrcv = r_msgrcv,
			nsignals = r_nsignals,
			nvcsw = r_nvcsw,
			nivcsw = r_nivcsw
		}
	poke ptr (RUsage utime stime maxrss ixrss idrss isrss minflt majflt nswap inblock oublock msgsnd msgrcv nsignals nvcsw nivcsw) = do
		(#poke struct rusage, ru_utime) ptr utime		
		(#poke struct rusage, ru_stime) ptr stime		
		(#poke struct rusage, ru_maxrss) ptr maxrss	
		(#poke struct rusage, ru_ixrss) ptr ixrss		
		(#poke struct rusage, ru_idrss) ptr idrss		
		(#poke struct rusage, ru_isrss) ptr isrss		
		(#poke struct rusage, ru_minflt) ptr minflt	
		(#poke struct rusage, ru_majflt) ptr majflt	
		(#poke struct rusage, ru_nswap) ptr nswap		
		(#poke struct rusage, ru_inblock) ptr inblock	
		(#poke struct rusage, ru_oublock) ptr oublock	
		(#poke struct rusage, ru_msgsnd) ptr msgsnd	
		(#poke struct rusage, ru_msgrcv) ptr msgrcv	
		(#poke struct rusage, ru_nsignals) ptr nsignals	
		(#poke struct rusage, ru_nvcsw) ptr nvcsw		
		(#poke struct rusage, ru_nivcsw) ptr nivcsw	

data Resuse = Resuse {
	wait_status :: Int,
	resource_usage :: RUsage,
	start_time :: Timeval,
	elapsed_time :: Timeval
} deriving (Show)

instance Storable Resuse where
	sizeOf		_ = (#size RESUSE)
	alignment	_ = alignment (undefined :: CDouble)
	peek ptr = do 
		rs_ws	<- (#peek RESUSE, waitstatus) ptr
		rs_ru 	<- (#peek RESUSE, ru) ptr
		rs_st	<- (#peek RESUSE, start) ptr
		rs_et	<- (#peek RESUSE, elapsed) ptr
		
		return Resuse {wait_status = rs_ws, resource_usage = rs_ru, start_time = rs_st, elapsed_time = rs_et}
	poke ptr (Resuse rs_ws rs_ru rs_st rs_et) = do
		(#poke RESUSE, waitstatus) ptr rs_ws
		(#poke RESUSE, ru) ptr rs_ru
		(#poke RESUSE, start) ptr rs_st
		(#poke RESUSE, elapsed) ptr rs_et
