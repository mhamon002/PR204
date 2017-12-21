#include "common_impl.h"




int do_socket(){
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(fd == -1){
    printf("SOCKET FAILURE : error %d : %s\n", errno, strerror);
    error("Failed to open listening socket");
    return -1;
  }
  return fd;
}

void init_serv_addr(struct sockaddr_in *serv_addr, int port){

  memset(serv_addr, '\0', sizeof(*serv_addr));
  serv_addr->sin_family = AF_INET;
  (* serv_addr).sin_port = htons( port );
  (* serv_addr).sin_addr.s_addr = INADDR_ANY;

}


int do_bind(int fd, struct sockaddr* addr){
  // addr.sin_family = AF_INET;
  if( bind(fd, addr, sizeof(struct sockaddr_in)) == -1){
    printf("BIND FAILURE : error %d, %s\n", errno, strerror(errno));
    error("Failed to bind");
    return -1;
  }
  return 0;
}

int do_listen(int fd, int maxconn){

  if (listen(fd, maxconn) == -1){
    printf("Error %d : %s",errno,strerror(errno));
    error("Failed to listen");
  }
}


int creer_socket_ecoute(struct sockaddr_in* addr_in, int *port_num)
{




  //initialisation addr




  addr_in->sin_family = AF_INET;
  //(* serv_addr).sin_port = htons( port_num );
  addr_in->sin_addr.s_addr = INADDR_ANY;
  addr_in->sin_port = htons(0);


  printf("%s\n",inet_ntoa(addr_in->sin_addr));




  struct addrinfo hints;
  struct addrinfo * result, *rp; //liste et element de la liste

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /*  socket */
//  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
//  hints.ai_protocol = IPPROTO_TCP;          /* Any protocol */
//  hints.ai_canonname = NULL;
//  hints.ai_addr = NULL;
//  hints.ai_next = NULL;

  char hostname[64];
  gethostname(hostname,64*sizeof(char));
  printf("%s\n",hostname);

  int s = getaddrinfo(hostname, NULL, &hints, &result);
  if (s != 0){
    printf("Erreur dans getaddrinfo : %s\n",gai_strerror(s));
    error("getaddrinfo failed.");
  }

  int fd;
  for (rp = result ;rp != NULL ;rp = rp->ai_next){
    //socket
    fd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
    if(fd == -1){
      printf("SOCKET FAILURE : error %d : %s\n", errno, strerror);
      continue;
    }
    if( bind(fd, rp->ai_addr, rp->ai_addrlen) == 0){
      unsigned int sock_len = sizeof(struct sockaddr_in);
      getsockname(fd,(struct sockaddr *) addr_in, &sock_len);

      break;
    }
    close(fd);
  }

  freeaddrinfo(result);

  if (rp == NULL){
    printf("BIND FAILURE : error %d, %s\n", errno, strerror(errno));
  }





  printf("%s\n",inet_ntoa(addr_in->sin_addr));
  //listen
  if (listen(fd, SOMAXCONN) == -1){
    printf("Error %d : %s",errno,strerror(errno));

  }


  //recuperation du numero de port
  *port_num = ntohs(addr_in->sin_port);


  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  /* renvoie le numero de descripteur */
  /* et modifie le parametre port_num */

  return fd;
}

int creer_socket_client(char* hostname_serv, char* port){
  /*int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(fd == -1){
    printf("SOCKET FAILURE : error %d : %s\n", errno, strerror);
    return -1;
  }
*/
  struct addrinfo hints;
  struct addrinfo * result, *rp; //liste et element de la liste

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Datagram socket */

  int s = getaddrinfo(hostname_serv, port, &hints, &result);
  if (s != 0){
    printf("Erreur dans getaddrinfo : %s\n",gai_strerror(s));
    error(strerror(errno));
  }

  int fd;
  for (rp = result ;rp != NULL ;rp = rp->ai_next){
    //socket
    fd = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
    if(fd == -1){
      printf("SOCKET FAILURE : error %d : %s\n", errno, strerror);
      continue;
    }
    if( connect(fd, rp->ai_addr, rp->ai_addrlen) != -1){
      printf("connecté\n");
      break;
    }
    close(fd);
    fd = -1;
  }

  freeaddrinfo(result);


  return fd;
}



int pipe_init(int pipefd[2]){
  if (-1 == pipe(pipefd)){
    printf("Pipe error : error %d, %s\n", errno, strerror(errno));
    ERROR_EXIT("pipe");
    return -1;
  }
  return 0;
}

void do_connect(int fd, char* ip_addr, int port){

    struct sockaddr_in sock_host;

    memset(&sock_host,'\0',sizeof(sock_host));
    sock_host.sin_family = AF_INET;
    inet_aton(ip_addr, &sock_host.sin_addr);

    if( connect(fd, (struct sockaddr*)&sock_host, sizeof(sock_host)) != 0 ){
      printf("connection failed error %d : %s\n",errno, strerror(errno));
      error("Failed to connect.");
    }


}

int do_accept(int socket, struct sockaddr* addr, socklen_t* addrlen){
  int fd_new = accept(socket, addr,addrlen);
  if (fd_new == -1){
    printf("ACCEPT FAILURE : error %d, %s\n", errno, strerror(errno));
    ERROR_EXIT("accept");
    return -1;
  }
  return fd_new;
}

ssize_t readline(int fd, char* str, size_t maxlen){
  int num = 0;
  do{
    num += read(fd,str+num,1);
  }
  while(num != maxlen && str[num-1] != '\n');

  return num;

}



ssize_t sendline(int fd, char* str, size_t maxlen){
  int sent = 0;
  do{
    sent += write(fd,str+sent,maxlen-sent);
  }
  while(sent != maxlen);

  return sent;

}


/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
