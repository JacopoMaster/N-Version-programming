#define _POSIX_SOURCE
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sig_handler(int signo) {
  (void)signo;
  kill(getppid(), SIGUSR1);
}

int main(void) {
  if (signal(SIGUSR1, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");
  // A long long wait so that we can easily issue a signal to this process
  while (1)
    sleep(1);
  return 0;
}