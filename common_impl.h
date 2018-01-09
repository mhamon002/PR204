#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
//#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define S_ECOUTE 0
#define S_PAS_ECOUTE 1
/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   char name[64];
   int port;
   int sock_fd; //socket entre les dsmwrap
   struct sockaddr_in* addr_in;
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
  char name[64];
  int init_sock_fd;
  int pipe_out_fd[2];
  int pipe_err_fd[2];
};
typedef struct dsm_proc dsm_proc_t;

int do_socket();

void init_serv_addr(struct sockaddr_in *serv_addr, int port);

int do_bind(int fd, struct sockaddr* addr);


int do_listen(int fd, int maxconn);

int creer_socket_ecoute(struct sockaddr_in* addr_in, int *port_num);

int creer_socket_client();

int pipe_init(int pipefd[2]);

void do_connect(int fd, char* ip_addr, int port);

int do_accept(int socket, struct sockaddr* addr, socklen_t* addrlen);

ssize_t readline(int fd, char* str, size_t maxlen);



ssize_t sendline(int fd, char* str, size_t maxlen);
