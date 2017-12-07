#include "common_impl.h"

int main(int argc, char **argv)
{

  printf("salut, moi c'est dsmwrap ,appelé depuis %s:%d\n",argv[1],atoi(argv[2]));
  int i = 0;
  while(argv[i] != NULL){

   printf("%d : %s\n",i,argv[i]);
    i++;
}

  char* hostname_serv = argv[1];
  char* host_port = argv[2];
  int exec_num = 3; //indice de la commande dans argv

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

   struct sockaddr_in *addr_in = malloc(sizeof(struct sockaddr_in));
   memset(addr_in, 0, sizeof(struct sockaddr_in));



   int sock_fd = creer_socket_client(hostname_serv, host_port);

   if(sock_fd == -1) error("creer_sock_client failed.");



//http://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
//   do_connect(sock_fd, host_addr, host_port);


   /* Envoi du nom de machine au lanceur */

   int hostname_size = 64;
   char hostname[hostname_size];

   gethostname(hostname, hostname_size);

   hostname_size = 0;
   while (hostname[++hostname_size] != '\0');

   char hostname_size_str[4];
   memset(hostname_size_str,'\0',4);
   sprintf(hostname_size_str,"%d",hostname_size);
   sendline(sock_fd, hostname_size_str, 4);


   sendline(sock_fd, hostname, hostname_size);

   /* Envoi du pid au lanceur */
   char pid_str[6] ;

   memset(pid_str,'\0',6);
   sprintf(pid_str,"%d",getpid());
   printf("j'envoie PID : %s\n", pid_str);
   sendline(sock_fd, pid_str, 6);


   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   struct sockaddr_in *listen_addr_in = malloc(sizeof(struct sockaddr_in));
   memset(listen_addr_in, 0, sizeof(struct sockaddr_in));
   int listen_port;

   int listen_fd = creer_socket_ecoute(listen_addr_in, &listen_port);

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   char port_str[7] ;
   memset(port_str,'\0',7);
   sprintf(port_str,"%d",listen_port);
   printf("j'envoie port : %s\n", port_str);
   sendline(sock_fd, port_str, 7);


   printf("Coucou c'est dsmwrap sur %s pid %d, je suis connecté\n",hostname,getpid());
   /*TODO : DSM */
   sleep(3);

   /* on execute la bonne commande */
   argv[argc] = NULL;
   execvp(argv[exec_num],argv + exec_num*sizeof(int));

   return 0;
}
