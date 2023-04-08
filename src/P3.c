#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#define MAX 80
#define SA struct sockaddr
#define SHARED_FILE "shared_file.txt"

long long sum(const char s[]);
int truncate(const char *path, off_t length);
int ftruncate(int fd, off_t length);
int client_sock_fd;
void handler(int sig) {
  (void)sig;  
  close(client_sock_fd);
  exit(0);
}
int main(int argc, char **argv) {
  if (argc != 4) {
    printf("P3> usage: %s MODE file_sem desc_PORT\n", argv[0]);
    exit(0);
  }
  signal(SIGINT, handler);
  // socket create and varification
  client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock_fd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("Socket successfully created..\n");
  struct sockaddr_in clientaddr;
  memset(&clientaddr, 0, sizeof(clientaddr));

  // assign IP, PORT
  clientaddr.sin_family = AF_INET;
  clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientaddr.sin_port = htons(atoi(argv[3]));

  // connect the client socket to server socket
  while (connect(client_sock_fd, (SA *)&clientaddr, sizeof(clientaddr)) != 0) {
    printf("P3: connection with the server failed...\n retry in 1 sec...\n");
    sleep(1);
  }
  printf("P3: connected.\n");
  char buff[16384];
  memset(buff, 0, 16384);
  long long sam = 0;
  // struct stat st;

  struct sembuf asem[1];
  asem[0].sem_num = 0;
  asem[0].sem_op = 0;
  asem[0].sem_flg = 0;

  srand(time(0));

  while (1) {
    
    asem[0].sem_op = -1;
    if (semop(atoi(argv[2]), asem, 1) == -1) {
      perror("semop: spool_signal_sem");
      exit(1);
    }


      FILE *shared_file = fopen(SHARED_FILE, "r");
      fgets(buff, 16384, shared_file);
      printf("P3>%s", buff);
    
      fclose(shared_file);
      if (buff[0] != '\n') {
        sam = sum(buff);
        printf("P3sum: %lld\n",sam);
      }
      if (strcmp(argv[1], "FALLIMENTO") == 0) {
        // Use current time as
        // seed for random generator
        //printf("here.\n");
        
        int num = rand() % 11;
        if (num >=9 && num <=9) {
          sam += 30;
        }
      }

      char b[MAX];
      memset(b, 0, MAX);
      sprintf(b, "%lld\n", sam);
      //
      //printf("what is going on?\n");
      //write(STDOUT_FILENO, b, strlen(b));
      int written = write(client_sock_fd, b, strlen(b));
      printf("P3 written %d\n", written);
      memset(buff, 0, 16384);
    }
 // }

  // close the socket
  close(client_sock_fd);
  return 0;
}

long long sum(const char s[]) {
  long long sum = 0;
  for (unsigned i = 0; i < strlen(s); ++i) {
    if (s[i] != ',') {
      sum += s[i];
    }
  }
  return sum;
}