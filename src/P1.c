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
#include <signal.h>
#define MAX 80
#define SA struct sockaddr

long long sum(const char s[]);
int client_sock_fd ;
void handler(int sig) {
  (void)sig;  
  close(client_sock_fd);
  exit(0);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("P1>usage: %s MODE desc_PORT\n", argv[0]);
    exit(0);
  }
  signal(SIGINT, handler);
  // socket create and varification
  client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock_fd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("P1 socket with decision_function successfully created..\n");
  struct sockaddr_in clientaddr;
  memset(&clientaddr, 0, sizeof(clientaddr));

  // assign IP, PORT
  clientaddr.sin_family = AF_INET;
  clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientaddr.sin_port = htons(atoi(argv[2]));

  // connect the client socket to server socket
  while (connect(client_sock_fd, (SA *)&clientaddr, sizeof(clientaddr)) != 0) {
    printf("P1: connection with the server failed...\n retry in 1 sec...\n");
    sleep(1);
  }
  
  printf("P1: connected.\n");
  char buff[16384];
  memset(buff, 0, 16384);
  int counter = 0;
  long long sam = 0;
  srand(time(0));
  while (read(STDIN_FILENO, buff + counter, 1)) {
    if (buff[counter] == '\n') {
      printf("P1>%s", buff);
      if (buff[0] != '\n') {
        sam = sum(buff);
        printf("P1sum: %lld\n",sam);
      }

      if (strcmp(argv[1], "FALLIMENTO") == 0) {
        // Use current time as
        // seed for random generator
        
        int num = rand() % 11;
        if (num >=4 && num <=4) {
          sam += 10;
        }
      }

      char b[MAX];
      memset(b, 0, MAX);
      sprintf(b, "%lld", sam);
      printf("P1=>%s\n", b);
      //client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
      //connect(client_sock_fd, (SA *)&clientaddr, sizeof(clientaddr));
      write(client_sock_fd, b, strlen(b));
      //close(client_sock_fd);
      memset(buff, 0, 16384);
      counter = 0;
    } else {
      counter++;
    }
  }

  // close the socket
  close(client_sock_fd);
  return 0;
}

long long sum(const char s[]) {
  char str[16384];
  char *token, *del = ",";
  long long sum = 0;

  strcpy(str, s);
  token = strtok(str, del);

  while (token != NULL) {
    char tk[16384];
    memset(tk, 0, 16384);
    sprintf(tk, "%s", token);
    for (unsigned long i = 0; i < strlen(tk); i++) {
      sum += tk[i];
    }
    token = strtok(NULL, del);
  }
  return sum;
}