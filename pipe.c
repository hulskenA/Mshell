/* mshell - a job manager */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
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
    /* Cas n */
    
  }
}
