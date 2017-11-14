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
    int fd1to2[2];
    int fd2to3[2];
    pipe(fd1to2);
    pipe(fd2to3);
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      close(fd1to2[0]);
      close(fd2to3[0]);
      close(fd2to3[1]);
      dup2(fd1to2[1],STDOUT_FILENO);
      close(fd1to2[1]);
      execvp(cmds[0][0],cmds[0]);
      exit(EXIT_FAILURE); /* shouldn't come back here */
      break;
    default:;
    }
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      close(fd1to2[1]);
      close(fd2to3[0]);
      dup2(fd1to2[0],STDIN_FILENO);
      dup2(fd2to3[1],STDOUT_FILENO);
      close(fd1to2[0]);
      close(fd2to3[1]);
      execvp(cmds[1][0],cmds[1]);
      exit(EXIT_FAILURE); /* shouldn't come back here */
      break;
    default:;
    }
    switch (fork()) {
    case -1:
      perror("error fork");
      exit(EXIT_FAILURE);
      break;
    case 0:
      close(fd1to2[0]);
      close(fd1to2[1]);
      close(fd2to3[1]);
      dup2(fd2to3[0],STDIN_FILENO);
      close(fd2to3[0]);
      execvp(cmds[2][0],cmds[2]);
      exit(EXIT_FAILURE); /* shouldn't come back here */
      break;
    default:;
    }
    close(fd1to2[0]);
    close(fd1to2[1]);
    close(fd2to3[0]);
    close(fd2to3[1]);
    wait(NULL);
    wait(NULL);
    wait(NULL);
  }
  else {
    int fd_n[nbcmd-1][2], i, j;
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
      close(fd_n[nbcmd-1][1]);
      for (i=0;i<nbcmd-2;i++) {
	close(fd_n[i][0]);
	close(fd_n[i][1]);
      }
      dup2(fd_n[nbcmd-1][0],STDIN_FILENO);
      close(fd_n[nbcmd-1][0]);
      execvp(cmds[nbcmd-1][0],cmds[nbcmd-1]);
      exit(EXIT_FAILURE); /* shoudln't reach this code */
      break;
    default:;
    }
  }
}
