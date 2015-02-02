#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
typedef void (*sighandler_t)(int);
sighandler_t vtalrm_signal, alrm_signal;

int main(int argc, char const *argv[])
{
	vtalrm_signal = signal (SIGVTALRM, SIG_IGN);
  alrm_signal = signal (SIGALRM, SIG_IGN);
  
  struct timespec req={0};
  req.tv_sec = 5;
  req.tv_nsec = 0;

  while (nanosleep(&req, &req) == -1) {
    printf("Got interrupt, continuing\n");
    continue;
    }
    printf("Slept\n");
    sleep(5);
    printf("Finishing!\n");
	return 0;
}