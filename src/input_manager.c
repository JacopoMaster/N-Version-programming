#define PORT "8880"
#define PORT_P1 "8770"
#define _POSIX_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
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
#define MAX 80
#define SA struct sockaddr

#define SHARED_FILE "shared_file.txt"

pid_t pid_p1, pid_desc_func, pid_faulure_mamager, pid_watchdog, pid_p2, pid_p3;
int client_sock_fd ;

void handler1(int sig) {
  (void)sig;
  fflush(stdout);
  kill(pid_p1, SIGINT);
  kill(pid_desc_func, SIGINT);
  kill(pid_faulure_mamager, SIGINT);
  kill(pid_p2, SIGINT);
  close(client_sock_fd);
  exit(0);
}

void handler(int sig) {
  (void)sig;
  fflush(stdout);
  kill(pid_p1, SIGINT);
  kill(pid_p2, SIGINT);
  kill(pid_p3, SIGINT);
  kill(pid_desc_func, SIGINT);
  kill(pid_faulure_mamager, SIGINT);
  kill(pid_watchdog, SIGINT);
  close(client_sock_fd);
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGUSR1, handler1);
  signal(SIGINT, handler);
  if (argc != 3) {
    printf("usage: $%s NORMALE path_to_dataset.csv\n", argv[0]);
    exit(0);
  }
  if (strcmp(argv[1], "NORMALE") != 0 && strcmp(argv[1], "FALLIMENTO")) {
    printf("unknown mode: %s\n", argv[1]);
    printf("modes are NORMALE and FALLIMENTO\n");
    exit(0);
  }
  int p[2];
  pipe(p);
  if (pipe(p) == -1) {
    perror("pipe");
    exit(1);
  }

  if ((pid_p1 = fork()) == -1) {
    perror("pid_p1");
    exit(1);
  } else if (pid_p1 == 0) {
    // close write end of pipe
    close(p[1]);
    // make 0 same as read-from end of pipe
    dup2(p[0], STDIN_FILENO);
    // close excess fildes
    close(p[0]);

    char *args[] = {"./P1", argv[1],PORT, NULL};
    execvp(args[0], args);
    // still around?  exec failed
    perror("./P1");
    exit(1);
  } else {

    if ((pid_faulure_mamager = fork()) == -1) {
      perror("pid_faulure_mamager");
      exit(1);
    } else if (pid_faulure_mamager == 0) {
      char *args[] = {"./failure_manager", argv[1], NULL};
      execvp(args[0], args);
      perror("./failure_manager"); /* still around?  exec failed */
      exit(1);

    } else {

      if ((pid_watchdog = fork()) == -1) {
        perror("pid_watchdog");
        exit(1);
      } else if (pid_watchdog == 0) {
        char buff[80];
        sprintf(buff, "%d", pid_faulure_mamager);
        char *args[] = {"./watchdog", buff, NULL};
        execvp(args[0], args);
        perror("./watchdog"); /* still around?  exec failed */
        exit(1);
      } else {

        // ipc semaphore to synchronize with decision maker
        // when decision signals when it receives a value
        // input manager waits for 3 signals at end of loop

        //  mutual exclusion semaphore, mutex_sem with an initial value 1.
        /* generate a key for creating semaphore  */
        key_t s_key;
        if ((s_key = ftok("./decision_function", 'a')) == -1) {
          perror("ftok");
          exit(1);
        }
        int three_sync_sem;
        if ((three_sync_sem = semget(s_key, 1, 0660 | IPC_CREAT)) == -1) {
          perror("semget");
          exit(1);
        }

        union semun {
          int val;
          struct semid_ds *buf;
          unsigned short array[1];
        } sem_attr;
        // Giving initial value.
        sem_attr.val = 1; // unlocked
        if (semctl(three_sync_sem, 0, SETVAL, sem_attr) == -1) {
          perror("semctl SETVAL");
          exit(1);
        }

        if ((pid_desc_func = fork()) == -1) {
          perror("pid_desc_func");
          exit(1);
        } else if (pid_desc_func == 0) {
          char failure_manager[80];
          sprintf(failure_manager, "%d", pid_faulure_mamager);
          char watchdog[80];
          sprintf(watchdog, "%d", pid_watchdog);
          char semaphore_key[80];
          sprintf(semaphore_key, "%d", three_sync_sem);
          char *args[] = {"./decision_function", failure_manager, watchdog,
                          argv[1], semaphore_key, PORT,NULL};
          execvp(args[0], args);
          perror("./decision_function"); /* still around?  exec failed */
          exit(1);

        } else {

          if ((pid_p2 = fork()) == -1) {
            perror("pid_p2");
            exit(1);
          } else if (pid_p2 == 0) {

            char *args[] = {"./P2", argv[1], PORT_P1, PORT, NULL};
            execvp(args[0], args);
            perror("./P2"); /* still around?  exec failed */
            exit(1);

          } else {

            //
        key_t P3_key;
        if ((P3_key = ftok("./P3", 'a')) == -1) {
          perror("ftok");
          exit(1);
        }
        int file_sync_sem;
        if ((file_sync_sem = semget(P3_key, 1, 0660 | IPC_CREAT)) == -1) {
          perror("semget");
          exit(1);
        }

        union semun {
          int val;
          struct semid_ds *buf;
          unsigned short array[1];
        } sem_file_attr;
        // Giving initial value.
        sem_file_attr.val = 0; // unlocked
        if (semctl(file_sync_sem, 0, SETVAL, sem_file_attr) == -1) {
          perror("semctl SETVAL");
          exit(1);
        }
        //

            if ((pid_p3 = fork()) == -1) {
              perror("pid_p3");
              exit(1);
            } else if (pid_p3 == 0) {
              char file_sem_buf[MAX];
              memset(file_sem_buf, 0, MAX);
              sprintf(file_sem_buf, "%d", file_sync_sem);
              char *args[] = {"./P3", argv[1], file_sem_buf,PORT,NULL};
              execvp(args[0], args);
              perror("./P3"); /* still around?  exec failed */
              exit(1);

            } else {

              // socket create and varification
              client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
              if (client_sock_fd == -1) {
                printf("socket creation failed...\n");
                exit(0);
              } else {
                printf("Socket with P2 successfully created..\n");
              }
              struct sockaddr_in clientaddr;
              memset(&clientaddr, 0, sizeof(clientaddr));

              // assign IP, PORT
              clientaddr.sin_family = AF_INET;
              clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
              clientaddr.sin_port = htons(atoi(PORT_P1));

              // connect the client socket to server socket
              while (connect(client_sock_fd, (SA *)&clientaddr,
                             sizeof(clientaddr)) != 0) {
                printf("connection with the P2 server failed...\n retry in 1 "
                       "sec...\n");
                sleep(1);
              }

              // close read end of pipe

              close(p[0]);
              // make STDOUT_FILENO same as write-to end of pipe
              dup2(p[1], STDOUT_FILENO);
              // close excess p
              close(p[1]);
              //
              struct sembuf asem[1];
                asem[0].sem_num = 0;
                asem[0].sem_op = 0;
                asem[0].sem_flg = 0;
              // open file
              FILE *csv = fopen(argv[2], "r");
              char buff[16384];
              // and discard first line:
              fgets(buff, 16384, csv);
              while (fgets(buff, 16384, csv)) {
                fprintf(stdout, "%s", buff);
                write(client_sock_fd, buff, sizeof(buff));
                //
                FILE *shared_file = fopen(SHARED_FILE, "w+");
                fprintf(shared_file, "%s", buff);
                fflush(shared_file);
                fclose(shared_file);

                asem[0].sem_op = 1;
                if (semop(file_sync_sem, asem, 1) == -1) {
                  perror("semop: buffer_count_sem");
                  exit(1);
                }
                
                memset(buff, 0, 16384);
                sleep(1);
                //printf("waiting for them to finish\n");

                asem[0].sem_op = -3;
                if (semop(three_sync_sem, asem, 1) == -1) {
                  perror("semop: buffer_count_sem");
                  exit(1);
                }
              }
            }
          }
        }
        //
      }
    }
  }

  return 0;
}