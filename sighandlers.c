/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>


#include "jobs.h"
#include "common.h"
#include "sighandlers.h"

/*
 * wrapper for the sigaction function
 */
int sigaction_wrapper(int signum, handler_t * handler) {
  struct sigaction sa;

  sa.sa_handler = handler;
  assert(sigemptyset(&sa.sa_mask) != -1);
  sa.sa_flags = SA_RESTART;
  assert(sigaction(signum, &sa, NULL) != -1);

  return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {
  int status,rsig;
  struct job_t *job;
  pid_t pid;
  sig=sig;
  
  if (verbose) {
    printf("[INFO] sigchld_handler: entering\n");
    printf("[INFO] sigchld_handler: getting pid of child...\n");
  }

  while ((pid=waitpid(-1,&status,WNOHANG | WUNTRACED))>0) {
    if (verbose)
      printf("[INFO] sigchld_handler: sucessfully got pid (pid:%d)!\n",pid);
    job = jobs_getjobpid(pid);
    
    if (WIFEXITED(status)) {
      /* Exit normale */
      if (verbose)
	printf("[INFO] sigchld_handler: a job has exited normally\n");
      jobs_deletejob(pid);
    }
    else if (WIFSTOPPED(status)) {
      if (verbose)
	printf("[INFO] sigchld_handler: a job has been stopped\n");
      /* Le fils a ete stoppe */
      if (job!=NULL) {
	if (verbose)
	  printf("[INFO] sigchld_handler: updating jb_state in list\n");;
	job->jb_state = ST;
      } else
	if (verbose)
	  printf("[INFO] sigchld_handler: ignoring because a process in a pipe group\n");
    }
    else if (WIFSIGNALED(status)) {
      /* Le fils a recu un signal */
      rsig = WTERMSIG(status);
      if (verbose)
	printf("[INFO] sigchld_handler: child recieved signal %d!\n",sig);
      if (rsig == SIGKILL) {
	if (verbose)
	  printf("[INFO] sigchld_handler: child recieved SIGKILL\n");
	jobs_deletejob(pid);
      }
      if (rsig == SIGTERM) {
	jobs_deletejob(pid);
      }
      if (rsig == SIGINT) {
	jobs_deletejob(pid);
      }
    }
    else 
      unix_error("[INFO] sigchld_handler: unknown case\n");
  }
  if (verbose)
    printf("[INFO] sigchld_handler: exiting\n");
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
  pid_t pid;
  struct job_t *job;
  sig=sig;
  
  if (verbose){
    printf("[INFO] sigint_handler: entering\n");
    printf("[INFO] sigint_handler: getting pid of fg process...\n");
  }

  if ((pid = jobs_fgpid()) > 0) {
    if (verbose)
      printf("[INFO] sigint_handler: sucessfully got pid of fg : %d\n",pid);
    job = jobs_getjobpid(pid);
    if (kill((contains_pipe(job->jb_cmdline)?-pid:pid), SIGINT) > -1) {
      if (verbose)
	printf("[INFO] sigint_handler: sucessfully sent SIGINT to fg process!\n");
    }
    else 
      unix_error("[ERROR] sigint_handler: error sending SIGINT to fg");
  }

  if (verbose)
    printf("[INFO] sigint_handler: exiting\n");
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {
  pid_t pid;
  struct job_t *job;
  sig=sig;

  
  if (verbose) {
    printf("[INFO] sigtstp_handler: entering\n");
    printf("[INFO] sigtstp_handler: getting foreground process id...\n");
    fflush(stdout);
  }

  if ((pid = jobs_fgpid()) > 0) {
    if (verbose) {
      printf("[INFO] sigtstp_handler: got pid(%d) of fg process!\n",pid);
      fflush(stdout);
    }
    job = jobs_getjobpid(pid);
    if (contains_pipe(job->jb_cmdline)){
      if (verbose) {
	printf("[INFO] sigtstp_handler: process uses pipes!!\n");
	fflush(stdout);
      }
      pid = -pid;
    }
    if (kill(pid, SIGTSTP) > -1) {
      if (verbose)
	printf("[INFO] sigtstp_handler: sucessfully sent SIGTSTP\n");
    }
    else
      unix_error("[ERROR] sigtstp_handler: error sending SIGTSTP to process\n");
  }

  if (verbose)
    printf("[INFO] sigtstp_handler: exiting\n");

}
