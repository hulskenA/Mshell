/* mshell - a job manager */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jobs.h"
#include "cmd.h"

void do_help() {
    printf("available commands are:\n");
    printf(" exit - cause the shell to exit\n");
    printf(BOLD "\t exit\n" NORM);
    printf(" jobs - display status of jobs in the current session\n");
    printf(BOLD "\t jobs\n" NORM);
    printf(" fg   - run a job identified by its pid or job id in the foreground\n");
    printf(BOLD "\t fg " NORM "pid" BOLD "|" NORM "jobid \n");
    printf(" bg   - run a job identified by its pid or job id in the background\n");
    printf(BOLD "\t bg " NORM "pid" BOLD "|" NORM "jobid \n");
    printf(" stop - stop a job identified by its pid or job id\n");
    printf(BOLD "\t stop " NORM "pid" BOLD "|" NORM "jobid \n");
    printf(" kill - kill a job identified by its pid or job id\n");
    printf(BOLD "\t kill " NORM "pid" BOLD "|" NORM "jobid \n");
    printf(" help - print this message\n");
    printf(BOLD "\t help\n" NORM);
    printf("\n");
    printf("ctrl-z and ctrl-c can be used to send a SIGTSTP and a SIGINT, respectively\n\n");
}

int contains_pipe(char *line) {
  char c;
  while ((c=(*(line++))))
    if (c=='|')
      return 1;
  return 0;
}

/* treat_argv - Determine pid or jobid and return the associated job structure */
struct job_t *treat_argv(char **argv) {
    struct job_t *jobp = NULL;

    /* Ignore command if no argument */
    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return NULL;
    }

    /* Parse the required PID or %JID arg */
    if (isdigit((int) argv[1][0])) {
        pid_t pid = atoi(argv[1]);
        if (!(jobp = jobs_getjobpid(pid))) {
            printf("(%d): No such process\n", (int) pid);
            return NULL;
        }
    } else if (argv[1][0] == '%') {
        int jid = atoi(&argv[1][1]);
        if (!(jobp = jobs_getjobjid(jid))) {
            printf("%s: No such job\n", argv[1]);
            return NULL;
        }
    } else {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return NULL;
    }

    return jobp;
}

/* do_bg - Execute the builtin bg command */
void do_bg(char **argv) {
  struct job_t* job = treat_argv(argv);
  pid_t pid;
  /* If there are no jobs*/
  if (job==NULL) return;
  pid = job->jb_pid;
  if (contains_pipe(job->jb_cmdline))
    pid = -pid; /* man kill(2) */
  if(kill(pid,SIGCONT)>-1)
    job->jb_state = BG;
  else
    unix_error("do_bg: error sending SIGCONT to child");
}

/* waitfg - Block until process pid is no longer the foreground process */
void waitfg(pid_t pid) {
  struct job_t *job = jobs_getjobpid(pid);
  while (job->jb_state == FG) {
    if (verbose)
      printf("[INFO] waitfg: still waiting...\n");
    sleep(1);
  }
}

/* do_fg - Execute the builtin fg command */
void do_fg(char **argv) {
  struct job_t * job = treat_argv(argv);
  pid_t pid;
  /* If there are no jobs*/
  if (job==NULL) return;
  pid = job->jb_pid;
  if (contains_pipe(job->jb_cmdline))
    pid = -pid;
  if(kill(pid,SIGCONT)>-1) {
    job->jb_state = FG;
    waitfg(job->jb_pid);
  } else 
    unix_error("do_fg: error sending SIGCONT to child");
}

/* do_stop - Execute the builtin stop command */
void do_stop(char **argv) {
  struct job_t * job = treat_argv(argv);
  pid_t pid;
  if (job==NULL) return;
  pid = job->jb_pid;
  if (contains_pipe(job->jb_cmdline))
    pid = -pid;
  if(kill(pid,SIGTSTP)<0)
    perror("kill error");;
}

/* do_kill - Execute the builtin kill command */
void do_kill(char **argv) {
  struct job_t * job = treat_argv(argv);
  pid_t pid;
  if (job==NULL) return;
  pid = job->jb_pid;
  if (contains_pipe(job->jb_cmdline))
    pid = -pid;
  if(kill(pid,SIGKILL)<0)
    unix_error("[ERROR] do_kill: error sending SIGKILL to child");
}

/* do_exit - Execute the builtin exit command */
void do_exit() {
  struct job_t* job;
  pid_t pid;
  if (verbose) {
    printf("[INFO] do_exit: entering\n");
    printf("[INFO] do_exit: killing all stopped jobs...\n");
  }
  while((job=jobs_getstoppedjob())) {
    /* Reap all the stopped child processes before suiciding */
    if (verbose){
      printf("[INFO] do_exit: got a job (pid:%d)\n",pid);
    }
    if (contains_pipe(job->jb_cmdline)) {
      if (verbose)
	printf("[INFO] do_exit: pipe group job\n");
      pid = -job->jb_pid;
    }
    else {
      if (verbose)
	printf("[INFO] do_exit: regular job\n");
      pid = job->jb_pid;
    }
    if (kill(pid,SIGKILL)<0)
      unix_error("[ERROR] do_exit: falied to send SIGKILL\n");
  }
  exit(EXIT_SUCCESS);
}

/* do_jobs - Execute the builtin jobs command */
void do_jobs() {
  jobs_listjobs();
}
