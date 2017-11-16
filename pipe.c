/* mshell - a job manager */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
#include "pipe.h"
#include "cmd.h"
#include "jobs.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
  int fd_n[MAXCMDS][2], i, j;
  jobs_addjob(getpid(),( bg==1 ? BG : FG),**cmds);
  for (i=0;i<nbcmd-1;i++)
    pipe(fd_n[i]);

  /* premier fork */
  switch (fork()) {
  case -1:
    perror("error fork");
    exit(EXIT_FAILURE);
    break;
  case 0:
    /* code */
    close(fd_n[0][0]);
    for (i=1;i<nbcmd-1;i++) {
      close(fd_n[i][0]);
      close(fd_n[i][1]);
    }
    dup2(fd_n[0][1],STDOUT_FILENO);
    close(fd_n[0][1]);
    execvp(cmds[0][0],cmds[0]);
    exit(EXIT_FAILURE); /* shouldn't reach this code */
    break;
  default:;
  }

  /* fork intermediere 
   * on fait nbcmd-2 fork intermedieres */
  for (i=1;i<nbcmd-1;i++) {
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      /* code */
      for (j=0;j<i-1;j++) {
	close(fd_n[j][0]);
	close(fd_n[j][1]);
      } 
      close(fd_n[i-1][1]);
      close(fd_n[i][0]);
      for (j=i+1;j<nbcmd;j++) {
	close(fd_n[j][0]);
	close(fd_n[j][1]);
      }
      dup2(fd_n[i-1][0],STDIN_FILENO);
      dup2(fd_n[i][1],STDOUT_FILENO);
      close(fd_n[i-1][0]);
      close(fd_n[i][1]);
      execvp(cmds[i][0],cmds[i]);
      exit(EXIT_FAILURE); /* shouldn't reach here */
      break;
    default :;
    }
  }
    
  /* dernier fork */
  switch (fork()) {
  case -1:
    perror("error fork");
    exit(EXIT_FAILURE);
    break;
  case 0:
    /* code */
    close(fd_n[nbcmd-2][1]);
    for (i=0;i<nbcmd-2;i++) {
      close(fd_n[i][0]);
      close(fd_n[i][1]);
    }
    dup2(fd_n[nbcmd-2][0],STDIN_FILENO);
    close(fd_n[nbcmd-2][0]);
    execvp(cmds[nbcmd-1][0],cmds[nbcmd-1]);
    exit(EXIT_FAILURE); /* shoudln't reach this code */
    break;
  default:;
  }

  for (i=0;i<nbcmd-1;i++) {
    close(fd_n[i][0]);
    close(fd_n[i][1]);
  }
  if (!bg) {
    waitfg(getpid());
  }
}
