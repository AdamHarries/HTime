/* `time' utility to display resource usage of processes.
   Copyright (C) 1990, 91, 92, 93, 96 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* Originally written by David Keppel <pardo@cs.washington.edu>.
   Heavily modified by David MacKenzie <djm@gnu.ai.mit.edu>.  */

extern const char *version_string;

#include "wait.h"
#include <stdio.h>
#include <sys/param.h>		/* For getpagesize, maybe.  */
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#ifndef errno
extern int errno;
#endif
#include <getopt.h>
#include "port.h"
#include "resuse.h"
#include "getpagesize.h"

// void error PARAMS((int status, int errnum, char *message, ...));

static void usage PARAMS((FILE *, int));

/* A Pointer to a signal handler.  */
// typedef RETSIGTYPE (*sighandler) ();
typedef void (*sighandler_t)(int);

/* msec = milliseconds = 1/1,000 (1*10e-3) second.
   usec = microseconds = 1/1,000,000 (1*10e-6) second.  */

/* Systems known to fill in the average resident set size fields:
   SunOS 4.1.3 (m68k and sparc)
   Mt. Xinu 4.3BSD on HP9000/300 (m68k)
   Ultrix 4.4 (mips)
   IBM ACIS 4.3BSD (rt)
   Sony NEWS-OS 4.1C (m68k)

   Systems known to not fill them in:
   OSF/1 1.3 (alpha)
   BSD/386 1.1 (anything derived from NET-2)
   NetBSD 1.0 (4.4BSD-derived)
   Irix 5.2 (R4000)
   Solaris 2.3
   Linux 1.0

   It doesn't matter how many clock ticks/second there are on
   systems that don't fill in those fields.

   If the avgresident (%t) we print is less than a power of 2 away from
   the maxresident (%M), then we likely are using the right number.
   Another good check is comparing the average text size with the
   output of `size' on the executable.

   According to the SunOS manual, there are 50 ticks/sec on the sun3
   and 100 on the sun4.

   Some manuals have an apparent error, claiming that units for average
   sizes are kb*sec.  Judging by the contents of `struct rusage', it
   looks like it should be kb*ticks, like on SunOS.  Ticks/sec seems
   to be (empirically):
   50 Mt. Xinu
   250 Ultrix (mips)
   50 ACIS
   100 NEWS-OS

   sysconf(_SC_CLK_TCK) is *unrelated*.  */

#if defined(sun3) || defined(hp300) || defined(ibm032)
#define TICKS_PER_SEC 50
#endif
#if defined(mips)
#define TICKS_PER_SEC 250
#endif
#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 100
#endif
       

/* The number of milliseconds in one `tick' used by the `rusage' structure.  */
#define MSEC_PER_TICK (1000 / TICKS_PER_SEC)

/* Return the number of clock ticks that occur in M milliseconds.  */
#define MSEC_TO_TICKS(m) ((m) / MSEC_PER_TICK)

typedef enum {false, true} boolean;

#define UL unsigned long
/* Return the number of kilobytes corresponding to a number of pages PAGES.
   (Actually, we use it to convert pages*ticks into kilobytes*ticks.)

   Try to do arithmetic so that the risk of overflow errors is minimized.
   This is funky since the pagesize could be less than 1K.
   Note: Some machines express getrusage statistics in terms of K,
   others in terms of pages.  */

static unsigned long
ptok (pages)
     unsigned long pages;
{
  static unsigned long ps = 0;
  unsigned long tmp;
  static long size = LONG_MAX;

  /* Initialization.  */
  if (ps == 0)
    ps = (long) getpagesize ();

  /* Conversion.  */
  if (pages > (LONG_MAX / ps))
    {				/* Could overflow.  */
      tmp = pages / 1024;	/* Smaller first, */
      size = tmp * ps;		/* then larger.  */
    }
  else
    {				/* Could underflow.  */
      tmp = pages * ps;		/* Larger first, */
      size = tmp / 1024;	/* then smaller.  */
    }
  return size;
}

/*
 * Takes a string, and returns an array of tokens (c strings) by breaking 
 * the string at any of the delimiter characters. 
 * Ignores/removes delimiter characters
 * 
 * 
 * Arguments:
 *    data: the string to split
 *    delimiter: the list of delimiters to split on
 *    token list: a pointer to an array of c strings, which is populated
 *          with the tokens split from the original string
 * Returns:
 *    The number of tokens the string has been split in to.
 * */
int cstring_to_token_array(char* data,char* delimiter,char*** token_list)
{
  char* token_iterator;
  char** output_array = malloc(sizeof(char*));
  // output_array = NULL;
  int index = 1;
  token_iterator = strtok(data,delimiter);
  while(token_iterator!=NULL)
  {
    char* buffer = calloc(strlen(token_iterator)+1,sizeof(char));
    strcpy(buffer, token_iterator);
    output_array = realloc(output_array, sizeof(char*)*index);
    output_array[index-1] = buffer;
    index++;
    token_iterator = strtok(NULL, delimiter);   
  }
  *token_list = output_array;
  return index-1;
}


/* Run command CMD and return statistics on it.
   Put the statistics in *RESP.  */

int run_command (char* cmd, RESUSE* resp)
{ 
  char ** ca_tokens;
  int tcount = cstring_to_token_array(cmd, " ", &(ca_tokens));
  for(int i = 0;i<tcount;i++)
  {
    printf("Str %d = %s\n",i,ca_tokens[i]);
  }
  ca_tokens = realloc(ca_tokens, sizeof(char*)*(tcount+1));
  ca_tokens[tcount] = (char*)NULL;

  pid_t pid;			/* Pid of child.  */
  sighandler_t interrupt_signal, quit_signal;

  resuse_start (resp);
  pid = fork ();		/* Run CMD as child process.  */
  if (pid < 0)
  {
    printf("Error, cannot fork");
    return 1;
  }
  else if (pid == 0)
    {				/* If child.  */
      /* Don't cast execvp arguments; that causes errors on some systems,
	 versus merely warnings if the cast is left off.  */
      execvp (ca_tokens[0], ca_tokens);
      printf("Error running: %s", cmd);
      // error (0, errno, "cannot run %s", cmd[0]);
      // _exit (errno == ENOENT ? 127 : 126);
      return errno;
    }  
  /* Have signals kill the child but not self (if possible).  */
  interrupt_signal = signal (SIGINT, SIG_IGN);
  quit_signal = signal (SIGQUIT, SIG_IGN);

  if (resuse_end (pid, resp) == 0)
  {
    printf("Error waiting for child process\n");
    // error (1, errno, "error waiting for child process");
    return 3;
  }
  /* Re-enable signals.  */
  signal (SIGINT, interrupt_signal);
  signal (SIGQUIT, quit_signal);
  return 0;
}

// void
// main (argc, argv)
//      int argc;
//      char **argv;
// {
//   const char **command_line;
//   RESUSE res;

//   command_line = getargs (argc, argv);
//   run_command (command_line, &res);
//   summarize (outfp, output_format, command_line, &res);
//   fflush (outfp);

//   if (WIFSTOPPED (res.waitstatus))
//     exit (WSTOPSIG (res.waitstatus));
//   else if (WIFSIGNALED (res.waitstatus))
//     exit (WTERMSIG (res.waitstatus));
//   else if (WIFEXITED (res.waitstatus))
//     exit (WEXITSTATUS (res.waitstatus));
// }

// static void
// usage (stream, status)
//      FILE *stream;
//      int status;
// {
//   fprintf (stream, "\
// Usage: %s [-apvV] [-f format] [-o file] [--append] [--verbose]\n\
//        [--portability] [--format=format] [--output=file] [--version]\n\
//        [--help] command [arg...]\n",
// 	   program_name);
//   exit (status);
// }
