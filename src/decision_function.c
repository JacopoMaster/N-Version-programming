#define _POSIX_SOURCE
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#define MAX 80
#define SA struct sockaddr

struct sembuf asem[1];
int three_sync_sem;
char mode[MAX];
FILE *system_log, *voted_output;
long long sums[3] = {0};
int slot = 0;
int failure_pid, watchdog_pid;
void pre_desc();
void decision() {
  if (sums[0] > -1 && sums[1] > -1) {
    printf("decision: \n");
    //fprintf(stdout, "%lld %lld %lld \n", sums[0], sums[1], sums[2]);
    fprintf(voted_output, "%lld %lld %lld \n", sums[0], sums[1], sums[2]);
    fflush(voted_output);
    if (sums[0] == sums[1] || sums[2] == sums[1] || sums[0] == sums[2]) {

      fprintf(system_log, "SUCCESSO\n");
      fflush(system_log);
      kill(watchdog_pid, SIGUSR1);
    } else {
      fprintf(system_log, "FALLIMENTO\n");
      fflush(system_log);
      printf("FALLIMENTO: %lld %lld %lld \n", sums[0], sums[1], sums[2]);
      // SIGUSR1 signal to Failure Manager.
      kill(failure_pid, SIGUSR1);
    }
    //
    sums[0] = -1;
    sums[1] = -1;
    sums[2] = -1;

    asem[0].sem_op = 3;
    if (semop(three_sync_sem, asem, 1) == -1) {
      perror("semop: mutex_sem");
      exit(1);
    }

  }
}
int sock_server_fd;
void handler(int sig) {
  (void)sig;  
  close(sock_server_fd);
  exit(0);
}
// Driver function
int main(int argc, char **argv) {

  if (argc != 6) {
    printf("usage: %s failure_pid watchdog_pid  mode  sem_key desc_PORT\n", argv[0]);
    exit(0);
  }
  signal(SIGINT, handler);
   asem[0].sem_num = 0;
  asem[0].sem_op = 0;
  asem[0].sem_flg = 0;

  three_sync_sem = atoi(argv[4]);
  srand(time(0));

  failure_pid = atoi(argv[1]);
  watchdog_pid = atoi(argv[2]);

  memset(mode, 0, MAX);
  strcpy(mode, argv[3]);
  system_log = fopen("system_log.txt", "w+");
  voted_output = fopen("voted_output.txt", "w+");

  // socket create and verification
  sock_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_server_fd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");

  struct sockaddr_in servaddr, cli;
  memset(&servaddr, 0, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[5]));

  // Binding newly created socket to given IP and verification
  if ((bind(sock_server_fd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
    printf("decision_function: socket bind failed...\n");
    exit(0);
  } else
    printf("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen(sock_server_fd, 5)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  } else
    printf("Server listening..\n");
  unsigned len = sizeof(cli);
  fd_set read_set;
  int clients[3] = {0};
  int num_client = 0;
  for (;;) {
    FD_ZERO(&read_set);
    FD_SET(sock_server_fd, &read_set);
    int maxfd = sock_server_fd;
    for (int i = 0; i < num_client; ++i){
      FD_SET(clients[i], &read_set);
      if (clients[i]>maxfd){
        maxfd=clients[i];
      }
    }
    printf("XXXXwaiting for select...XXXXX\n");
    int activity =
        select(maxfd + 1, &read_set, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) {
      printf("select error");
      exit(1);
    }

    if (FD_ISSET(sock_server_fd, &read_set)) {
      // Accept the data packet from client and verification
      if (num_client < 3) {
        clients[num_client] = accept(sock_server_fd, (SA *)&cli, &len);
        if (clients[num_client] < 0) {
          printf("server accept failed...\n");
          exit(0);
        } else {
          printf("decision_function accepted %d \n",clients[num_client]);
          FD_SET(clients[num_client], &read_set);
          num_client++;
        }
      } else {
        //printf("more than 3.\n");
      }
    } else {
      for (int i = 0; i < num_client; ++i) {
        if (FD_ISSET(clients[i], &read_set)) {
          printf("func2: %d \n",clients[i]);
          char buff[MAX];
          memset(buff, 0, MAX);
          // read the message from client and copy it in buffer
          int read_data=read(clients[i], buff, MAX);
          printf("desc=>received: %s slot: %d len: %d\n", buff,slot,read_data);
          sums[slot] = atoll(buff);
          if (slot == 2) {
            decision();
            slot = 0;
          }else{
            slot++;
          }
 
          
        }
      }
    }
  }

  // After chatting close the socket
  close(sock_server_fd);
}
void pre_desc() {
  if (strcmp(mode, "FALLIMENTO") == 0) {
    int num = rand() % 11;
    if (num == 5) {
      sums[1] += 20;
    }

    int num2 = rand() % 11;
    if (num2 == 6) {
      sums[2] += 30;
    }

    int num3 = rand() % 11;
    if (num3 == 7) {
      sums[0] += 10;
    }
  }
}
