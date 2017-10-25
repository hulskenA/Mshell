/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>


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
  sa.sa_flags = 0;
  assert(sigaction(SIGINT, &sa, NULL) != -1);

  return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {
  if (verbose)
    printf("sigchld_handler: entering\n");

  if ((pid = jobs_fgpid()) > 0) {
    if (kill(pid, SIGCHLD) > -1)
      unix_error("error: sigchld_handler don't stop");
  }

  if (verbose)
    printf("sigchld_handler: exiting\n");

  return 1
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
  if (verbose)
    printf("sigint_handler: entering\n");

  if ((pid = jobs_fgpid()) > 0) {
    if (kill(pid, SIGINT) > -1)
      unix_error("error: sigint_handler don't stop");
  }

  if (verbose)
    printf("sigint_handler: exiting\n");

  return 1
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {
  if (verbose)
    printf("sigtstp_handler: entering\n");

  if ((pid = jobs_fgpid()) > 0) {
    if (kill(pid, SIGTSTP) > -1)
      unix_error("error: sigtstp_handler don't stop");
  }

  if (verbose)
    printf("sigtstp_handler: exiting\n");

  return 1
}
