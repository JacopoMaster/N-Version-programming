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
int sock_server_fd ,client_sockf_fd;
void handler(int sig) {
  (void)sig;  
  close(sock_server_fd);
  close(client_sockf_fd);
  exit(0);
}
int main(int argc, char **argv) {
  if (argc != 4) {
    printf("P2>usage: %s MODE MY_PORT DESC_PORT\n", argv[0]);
    exit(0);
  }
  signal(SIGINT, handler);
  // socket create and varification
  client_sockf_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sockf_fd == -1) {
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
  while (connect(client_sockf_fd, (SA *)&clientaddr, sizeof(clientaddr)) != 0) {
    printf("P2: connection with the server failed...\n retry in 1 sec...\n");
    sleep(1);
  }
  printf("P2: connected\n");
  //
  // socket create and verification
  sock_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_server_fd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  } else
    printf("P2 socket with decision_function successfully created..\n");

  struct sockaddr_in servaddr, cli;
  memset(&servaddr, 0, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[2]));

  // Binding newly created socket to given IP and verification
  if ((bind(sock_server_fd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
    printf("P2 %s bind failed...\n",argv[2]);
    exit(0);
  } else
    printf("P2 Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen(sock_server_fd, 5)) != 0) {
    printf("P2 Listen failed...\n");
    exit(0);
  } else
    printf("P2: listening..\n");
  unsigned len = sizeof(cli);
  //
  // Accept the data packet from client and verification
  int connfd = accept(sock_server_fd, (SA *)&cli, &len);
  if (connfd < 0) {
    printf("server accept failed...\n");
    exit(0);
  } else
    printf("server accept the client...\n");
  char buff[16384];
  memset(buff, 0, 16384);
  //int counter = 0;
  long long sam = 0;

  srand(time(0));

  while (read(connfd, buff, 16384)) {
    //if (buff[counter] == '\n') {
      printf("P2>%s", buff);
      if (buff[0] != '\n') {
        sam = sum(buff);
        printf("P2sum: %lld\n",sam);
      }
      if (strcmp(argv[1], "FALLIMENTO") == 0) {
        // Use current time as
        // seed for random generator
        
        int num = rand() % 11;
        if (num >=3 && num <= 3) {
          sam += 20;
        }
      }

      char b[MAX];
      memset(b, 0, MAX);
      sprintf(b, "%lld", sam);
      //printf("P2=>%s\n", b);
      write(client_sockf_fd, b, strlen(b));

      memset(buff, 0, 16384);
      //counter = 0;
    //} else {
      //counter++;
    //}
  }

  // close the socket
  close(client_sockf_fd);
  return 0;
}

long long sum(const char s[]) {
  long long sum = 0;
  for (int i = strlen(s); i >= 0; i--){
    if (s[i]!=','){
      sum+=s[i];
    }
  }
  return sum;
}