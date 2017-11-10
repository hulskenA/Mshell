/* mshell - a job manager */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include "pipe.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
  assert(bg==bg);
  if (nbcmd==2) {
    /* Cas 2 */
    int fd[2];
    pipe(fd);
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      close(fd[0]);
      dup2(fd[1],STDOUT_FILENO);
      close(fd[1]);
      execvp(cmds[0][0],cmds[0]);
      exit(EXIT_FAILURE);
      break;
    default:
      ;
    }
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      close(fd[1]);
      dup2(fd[0],STDIN_FILENO);
      close(fd[0]);
      execvp(cmds[1][0],cmds[1]);
      exit(EXIT_FAILURE);
      break;
    default:
      ;
    }
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
  }
  else {
    /* Cas 3 */
    /* Creation de n-1 pipes */
    int fd_n[nbcmd][2],i,j;
    printf("%d\n",nbcmd); /* DEBUG */
    for (i=0;i<nbcmd-1;i++)
      pipe(fd_n[i]);
    /* Fork n fois */
    for (i=0;i<nbcmd;i++) {
      printf("%d iter\n",i); /* DEBUG */
      switch (fork()) {
      case -1:
	perror("fork error");
	exit(EXIT_FAILURE);
	break;
      case 0:
	if (i==0) {
	  /* Cas de la premiere pipe */
	  for (j=0; j<nbcmd-1; j++)
	    if (j!=i) {
	      close(fd_n[j][0]);
	      close(fd_n[j][1]);
	    }
	  close(fd_n[i][0]);
	  /* Explication : On ferme tous les pipes
	   * sauf la premiere sur laquelle on veut
	   * pouvoir ecrire */
	  dup2(fd_n[i][1],STDOUT_FILENO);
	  /* On redirige les i/o proprement
	   * et on referme les autres flux qui
	   * qui sont maintenant inutiles */
	  close(fd_n[i][1]);
	}
	else if (i==nbcmd-1) {
	  /* Cas de la derniere pipe */
	  for (j=0; j<nbcmd-1; j++)
	    if (j!=i) {
	      close(fd_n[j][0]);
	      close(fd_n[j][1]);
	    }
	  close(fd_n[i][1]);
	  /* Explication : On ferme tous les pipes
	   * sauf la derniere sur laquelle on veut
	   * pouvoir lire */
	  dup2(fd_n[i][0],STDIN_FILENO);
	  /* On redirige les i/o proprement
	   * et on referme les autres flux qui
	   * qui sont maintenant inutiles */
	  close(fd_n[i][0]);
	}
	else {
	  for (j=0; j<nbcmd-1; j++)
	    if (j!=i && j!=(i-1)) {
	      close(fd_n[j][0]);
	      close(fd_n[j][1]);
	    }
	  close(fd_n[i][0]);
	  close(fd_n[i-1][1]);
	  /* Explication : On ferme tous les pipes
	   * sauf pour la i-eme et (i-1)-eme sur les-
	   * quelles on veut pouvoir ecrire et lire
	   * respectivement */
	  dup2(fd_n[i][1],STDOUT_FILENO);
	  dup2(fd_n[i-1][0],STDIN_FILENO);
	  /* On redirige les i/o proprement
	   * et on referme les autres flux qui
	   * qui sont maintenant inutiles */
	  close(fd_n[i][1]);
	  close(fd_n[i-1][0]);
	}
	execvp(cmds[i][0],cmds[i]);
	exit(EXIT_FAILURE);
	break;
      default:;
      }
    }
    for (i=0;i<nbcmd-1;i++) {
      close(fd_n[i][0]);
      close(fd_n[i][1]);
    }
    for (i=0; i<nbcmd;i++)
      /* On attend la fin de tous les fils
       * car le mshell ne redonne la mail tant
       * la commande n'as pas execute */
      wait(NULL);
  }
}
