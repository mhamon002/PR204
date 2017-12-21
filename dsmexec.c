#include "common_impl.h"

/* variables globales */
//dsm_proc_conn_t *tab_info_conn;


/* un tableau gerant les infos d'identification */
/* des processus dsm */
//dsm_proc_t *proc_array = calloc(20, sizeof(int)+sizeof(pid_t)+sizeof(char)*64+2*sizeof(int)*2);


//calloc(20,sizeof(dsm_procproc_array_t));

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
}

void* read_pipes(dsm_proc_t * dsm_proc){
    printf("thread pour %d\n",dsm_proc->connect_info.rank);
    char buf[256];
    int ret;
    do{
      //printf("thread pour %d, j'essaie de lire\n",dsm_proc->connect_info.rank);
      ret = readline(dsm_proc->pipe_out_fd[0],buf, 512);
      if (ret > 0)
        printf("machine %d : %s", dsm_proc->connect_info.rank,buf);
        fflush(stdout);
      if(ret == -1) printf("%s\n",strerror(errno));
      memset(buf,'\0',256);
    }
    while(ret != -1);

    printf("%d: %s\n", dsm_proc->connect_info.rank, strerror(errno));
    memset(buf,'\0',256);
    readline(dsm_proc->pipe_err_fd[0],buf, 256);
    printf("stderr dans le pipe %d : xx%sxx\n", dsm_proc->connect_info.rank,buf);
}

int main(int argc, char *argv[])
{
  char buf[256];
//  printf("%i\n",sizeof(dsm_proc_t));
  dsm_proc_t *proc_array = malloc(20*sizeof(dsm_proc_t));
  if (argc < 3){
    usage();
  }
  else {
    pid_t pid;
    int num_procs = 0;
    int i;



    //proc_array = calloc(20,sizeof(int)+sizeof(pid_t)+sizeof(char)*64+2*sizeof(int)*2);
    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    /* XXX.sa_handler = sigchld_handler; */

    /* lecture du fichier de machines */

    FILE* machine_file;
    machine_file = fopen(argv[1],"r");

    /* 1- on recupere le nombre de processus a lancer */
    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */

    /*
    while( fgets(proc_array[num_procs].name, 64, machine_file) != NULL ){
    num_procs++;
  }
  */
  int fini = 0;
  while(!fini){
    char buf[256];
    int i = 0;
    do {
      buf[i++] = fgetc(machine_file);
    }
    while((buf[i-1] != '\n') && (buf[i-1] != EOF));
    if (buf[i-1] == EOF) fini = 1;

    buf[i-1] = '\0';
    if (i>=2){
      strcat(proc_array[num_procs].name,buf);
      proc_array[num_procs].connect_info.rank = num_procs;

      num_procs++;
    }
  }
    /* creation de la socket d'ecoute */
    /* + ecoute effective */
    int listen_port;

    struct sockaddr_in *listen_sockaddr_in = malloc(sizeof(struct sockaddr_in));
    memset(listen_sockaddr_in, 0, sizeof(struct sockaddr_in));


    init_serv_addr(listen_sockaddr_in, 0);
    int listen_fd = do_socket();

    do_bind(listen_fd,(struct sockaddr*)listen_sockaddr_in);

    do_listen(listen_fd,SOMAXCONN);

    unsigned int sock_len = sizeof(struct sockaddr_in);
    getsockname(listen_fd,(struct sockaddr *) listen_sockaddr_in, &sock_len);


    listen_port = ntohs(listen_sockaddr_in->sin_port);
    printf("%d\n",listen_port);


    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      //if( i==1) printf("au debut de la boucle %d\n",i);
      //printf("%s\n",proc_array[i].name);
      /* creation du tube pour rediriger stdout */
      pipe_init(proc_array[i].pipe_out_fd);

      /* creation du tube pour rediriger stderr */
      pipe_init(proc_array[i].pipe_err_fd);

      //pid = 998;
      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

      //  fprintf(stdout,"================Coucou \n");

        /* redirection stdout */

        dup2(proc_array[i].pipe_out_fd[1], STDOUT_FILENO);
        close(proc_array[i].pipe_out_fd[0]);

        /* redirection stderr */
        dup2(proc_array[i].pipe_err_fd[1], STDERR_FILENO);
        close(proc_array[i].pipe_err_fd[0]);



        /* Creation du tableau d'arguments pour le ssh */
        char** newargv = malloc(7*sizeof(char*));
        newargv[0] = "ssh";
        newargv[1] = proc_array[i].name;
        newargv[2] = "/net/t/efaye/Desktop/PR204/Phase1/bin/dsmwrap";
      //  newargv[3] = inet_ntoa(listen_sockaddr_in->sin_addr);
        char hostname[64];
        gethostname(hostname,64*sizeof(char));
        newargv[3] = hostname;
        newargv[4] = malloc(7*sizeof(char));
        sprintf(newargv[4],"%d",listen_port);
        printf("aaaaaaaaa %s\n",newargv[4]);
        newargv[5] = argv[2];
        newargv[6] = NULL;

    //    printf("%s\n",newargv);
        // jump to new prog :


        execvp(newargv[0],newargv);
    //  execvp("echo",test);
        printf("exec raté : error %d, %s\n", errno, strerror(errno));
    //    printf("coucou\n");
        //
        int ret = 0;


        ret = write(proc_array[i].pipe_out_fd[1],"salut",10);
        if( -1 == ret)
          fprintf(stdout,"========== %s\n",strerror(errno));



      }
      else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */

        close(proc_array[i].pipe_out_fd[1]);
        close(proc_array[i].pipe_err_fd[1]);

        num_procs_creat++;
        printf("%d\n",proc_array[i].connect_info.rank);
        pthread_t thread;
        pthread_create(&thread, NULL, read_pipes, &(proc_array[i]));
/*
        int ret = 0;
        memset(buf,'\0',256);
        while(read(proc_array[i].pipe_out_fd[0],buf, 256)){
          sleep(2);
          printf("dans le pipe %d : xx%sxx\n", i,buf);
          memset(buf,'\0',256);
        }
        printf("%s\n",strerror(errno));
        ret = read(proc_array[i].pipe_out_fd[0],buf, 256);
        if(-1 == ret)
          fprintf(stdout,"========== %s\n",strerror(errno));
        printf("dans le pipe %d : xx%sxx\n", i,buf);
        memset(buf,'\0',256);
*/
      }
    }

    printf("num_procs : %d\n", num_procs);
    for(i = 0; i < num_procs ; i++){

      /* on accepte les connexions des processus dsm */
      struct sockaddr_in *tmp_addr_in = malloc(sizeof(struct sockaddr_in));
      memset(tmp_addr_in, 0, sizeof(struct sockaddr_in));

      socklen_t size = sizeof(struct sockaddr_in);
      printf("appel à accept\n");
      int tmp_fd = do_accept(listen_fd, (struct sockaddr*)tmp_addr_in, &size);
      printf("connecté à dsmwrap\n");
      /*  On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      char hostname_size_str[4];
      readline(tmp_fd, hostname_size_str, 4);
      printf("%s\n",hostname_size_str);
      /* 2- puis la chaine elle-meme */
      int hostname_size = atoi(hostname_size_str);



      char* tmp_hostname = malloc(hostname_size*sizeof(char));
      readline(tmp_fd, tmp_hostname, hostname_size);

      printf("%s\n",tmp_hostname);


      int cur_proc;
      int j;
      for (j=0 ; j <num_procs ; j++){
        if (strcmp(tmp_hostname, proc_array[j].name) == 0){
          proc_array[j].connect_info.init_sock_fd = tmp_fd;
          proc_array[j].connect_info.addr_in = tmp_addr_in;
          cur_proc = j;
        }
      }

      /* On recupere le pid du processus distant  */
      char pid_str[6];

      readline(proc_array[cur_proc].connect_info.init_sock_fd, pid_str, 6);

      printf("pid : %s\n", pid_str);

      proc_array[cur_proc].pid = atoi(pid_str);

      /* On recupere le numero de port de la socket */
      /* d'ecoute des processus distants */
      char port_str[7];
      readline(proc_array[cur_proc].connect_info.init_sock_fd, port_str, 7);

      printf("port : %s\n", port_str);

      proc_array[cur_proc].connect_info.addr_in->sin_port = htons(atoi(pid_str));

      printf("connecté à machine n° %d : %s, pid %d\n",cur_proc,proc_array[cur_proc].name,proc_array[cur_proc].pid);
    }

    /* envoi du nombre de processus aux processus dsm*/
    for (i = 0; i < num_procs; i++) {
        memset(buf, '\0', 256);
        sprintf(buf, "%d\n",num_procs);
        sendline(proc_array[i].connect_info.init_sock_fd, buf, strlen(buf));
    }

    /* envoi des rangs aux processus dsm */
    for (i = 0; i < num_procs; i++) {
        memset(buf, '\0', 256);
        sprintf(buf, "%d\n",proc_array[i].connect_info.rank);
        sendline(proc_array[i].connect_info.init_sock_fd, buf, strlen(buf));
    }

    /* envoi des infos de connexion aux processus */
    int j;
    for(i = 0; i < num_procs; i++){
      for(j = 0; j < num_procs; j++){
        memset(buf, 0, 256);
        sprintf(buf,"%s",proc_array[j].name);
        sendline(proc_array[i].connect_info.init_sock_fd, buf, strlen(buf)); // nom des machines
        memset(buf,0, 256);
        sprintf(buf, "%d", proc_array[j].connect_info.addr_in->sin_port);
        sendline(proc_array[i].connect_info.init_sock_fd, buf, strlen(buf)); // numéro ports
      }
    }


    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /* while(1)
    {
    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)



  };
  */

  /* on attend les processus fils */

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
  pause();
}
exit(EXIT_SUCCESS);
}
