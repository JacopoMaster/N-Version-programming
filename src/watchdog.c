#define _POSIX_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

int failure_pid;

void sig_handler(int signum) {
  signal(SIGUSR1, sig_handler);
   signal(SIGALRM, sig_handler);
  // signal handler for SIGINT
  if (signum == SIGUSR1) {
    //printf("\nSnoozing for 2 seconds....\n");
    alarm(2);
  }

  // signal handler for SIGALRM
  if (signum == SIGALRM) {

    // printf("Inside handler function for SIGALRM\n");
    kill(failure_pid, SIGUSR1);
  }
}

int main( int argc, char**argv ){

  if (argc != 2) {
    printf("usage: %s failure_pid \n", argv[0]);
    exit(0);
  }

  failure_pid = atoi(argv[1]);

  // Register signal handler for SIGALRM
  signal(SIGALRM, sig_handler);
  // Register signal handler SIGUSR1
  signal(SIGUSR1, sig_handler);

  alarm(5);  // Schedule the first alarm after 2 seconds
 
  while(1){
    pause(); // waiting until signal is handled
  }
 
return 0;
}