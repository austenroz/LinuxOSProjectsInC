/* 
   OS PROJECT 1c

   myshell - basic shell replacement

usage:

myshell

reads in a line of keyboard input at a time, parsing it into
tokens that are separated by white spaces (set by #define
SEPARATORS).

Prompt contains current working directory.  

Supports i/o redirection With output redirection, if the redirection character
is '>' then the outputfile is created if it does not exist and truncated if it
does. If the redirection token is '>>' then the outputfile is created if it does
not exist and appended to if it does.

The redirection symbols: '<', '>' & '>>' must be delimited from other command
line arguments by white space - one or more spaces and/or tabs.

Internal Commands/Aliases:

cd - change dirtectory (PWD environment variable updated)
clr - clear the screen using the system function clear
dir <directory> - list the current directory contents (ls -al <directory>)
environ - list all the environment strings
help - display help manual
quit - quit from the program with a zero return value
echo <string> - output string to the console
pause - pause the shell until enter is pressed



External Commands:

All other command line inputs are executed via a fork and exec

 *********************************************************************
version: 1.4
date:    November 2017
authors:  Dr Ian G Graham, ian.graham@griffith.edu.au
		  Austen Rozanski, austenrozanski@ou.edu
history:
v1.0:  Original Exercise 2 command set: clr, dir, environ, quit
v1.1:  Add cd command, PWD alteration, and working directory in prompt
syserrmsg (Exercise 3)
v1.2:  Add fork and exec capability via execute function (Exercise 4)
v1.3:  Add i/o redirection and shell status structure
v1.4:  Add echo and pause commands. Allow background execution.

Add help command and apply i/o redirection to aliases
 *******************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators
#define README "readme"                        // help file name

struct shellstatus_st {
	int foreground;                            // foreground execution flag
	char * infile;                             // input redirection flag & file
	char * outfile;                            // output redirection flag & file
	char * outmode;                            // output redirection mode
};
typedef struct shellstatus_st shellstatus;

extern char **environ;
void check4redirection(char **, shellstatus *);// check command line for i/o redirection
void errmsg(char *, char *);                   // error message printout
void execute(char **, shellstatus);            // execute command from arg array
FILE * redirected_op(shellstatus);             // return required o/p stream
void syserrmsg(char *, char *);                // system error message printout

/*******************************************************************/

int main (int argc, char ** argv)
{
	FILE * ostream = stdout;                   // (redirected) o/p stream
	char linebuf[MAX_BUFFER];                  // line buffer
	char cwdbuf[MAX_BUFFER];                   // cwd buffer
	char * args[MAX_ARGS];                     // pointers to arg strings
	char ** arg;                               // working pointer thru args
	char * prompt = "==>" ;                    // shell prompt
	char * readmepath;                         // readme pathname
	shellstatus status;                        // status structure
	int execargs;                              // execute command in args flag
	int i;                                     // working index

	FILE * fp;					// pointer to input file (stdin by default)
	int readingFromFile = FALSE;		// true if reading from file

	// check if file for input included in cmd line args.
	// if a file is specified in cmd line args, set readingFromFile
	// variable to true and read from the file instead of stdin.
	if (argc > 1) {
		readingFromFile = TRUE;
		if (argv[1]) {
			fp = fopen(argv[1], "r");

		}
		else {
			syserrmsg("error opening file.", NULL);
			return 1;
		}
	}
	// if there is no file specified, read from stdin
	else {
		fp = stdin;
	}

	// if an error occured while retrieving input stream 
	if (fp == NULL) {
		syserrmsg("error getting input stream.", NULL);
		return 1;
	}




	// get starting cwd to add to readme pathname

	if(!getcwd(cwdbuf, sizeof(cwdbuf))) {  // read current working directory
		syserrmsg("getcwd",NULL);
		cwdbuf[0] = 0;
	}
	strcat(cwdbuf,"/" README);      // make full readme pathname 
	readmepath = strdup(cwdbuf);

	// keep reading input until "quit" command or eof of redirected input

	while (!feof(fp)) {

		// (re)initialise status structure

		status.foreground = TRUE;

		// set up prompt

		if (!readingFromFile) {
			if(getcwd(cwdbuf, sizeof(cwdbuf)))   // read current working directory
				printf("%s%s", cwdbuf,prompt);   // output as prompt
			else
				printf("getcwd ERROR %s",prompt);
		}
		fflush(stdout);


		// get command line from input

		if (fgets(linebuf, MAX_BUFFER, fp )) {

			// tokenize the input into args array 

			arg = args;
			*arg++ = strtok(linebuf,SEPARATORS);   // tokenize input
			while ((*arg++ = strtok(NULL,SEPARATORS)));
			// last entry will be NULL 

			if (args[0]) {                     // if there's anything there

				execargs = TRUE;               // set default execute of args

				// check for i/o redirection

				check4redirection(args, &status);

				// check for background or foreground process

				for (int i = 0; i < MAX_ARGS && args[i]; i++) {
					if (!strcmp(args[i],"&")) {
						status.foreground = FALSE;
						args[i] = NULL;
						break;
					}
				}

				// check for internal/external commands

				// "cd" command
				if (!strcmp(args[0],"cd")) {
					if (!args[1]) {            // no directory argument
						if (getcwd(cwdbuf, sizeof(cwdbuf)))
							printf("%s\n", cwdbuf); // print cwd
						else
							syserrmsg("retrieving cwd",NULL);
					} else {
						if (chdir(args[1])) {  // else change directory 
							syserrmsg("change directory failed",NULL);
						} else {
							strcpy(cwdbuf,"PWD="); // & set environment value
							if ((getcwd(&cwdbuf[4], sizeof(cwdbuf)-4))) {
								if (putenv(strdup(cwdbuf)))
									syserrmsg("change of PWD environment variable failed",NULL);
							} else {
								syserrmsg("retrieving cwd",NULL);
							}
						}
					} 
					execargs = FALSE;           // no need for further exec
				} else
				// "clr" command                    
				if (!strcmp(args[0],"clr")) {  
					args[0] = "clear";
					args[1] = NULL;
				} else
				// "dir" command
				if (!strcmp(args[0],"dir")) {
					for (i = MAX_ARGS - 1; i > 1; i--) 
						args[i] = args[i-1];     // shunt arguments
					args[0] = "ls";
					args[1] = "-al";
					if (!args[2])
						args[2] = ".";           // if no arg set current directory
				} else
				// "environ" command                    
				if (!strcmp(args[0],"environ")) {
					ostream = redirected_op(status);
					char ** envstr = environ;
					while (*envstr)              // print out environment
						fprintf(ostream, "%s\n",*envstr++);
					if (ostream != stdout) fclose(ostream);
					execargs = FALSE;            // no need for further exec
				} else
				// "echo" command
				if (!strcmp(args[0],"echo")) {
					if (args[1]) {
						printf("%s ", args[1]);
					}
					for (int i = 2; i < MAX_ARGS && args[i]; i++) {
						printf(" %s", args[i]);
					}
					printf("\n");
					execargs = FALSE;
				} else
				// "pause" command
				if (!strcmp(args[0],"pause")) {
					printf("Press Enter to continue...");
					// in order to pause, use getchar()
					// the loop and putchar is to use up any characters
					// typed while paused.
					char ch;
					while ((ch = getchar()) != '\n') putchar(ch);
					fflush(stdin);
					printf("\n");
					execargs = FALSE;
				} else
				// "help" command
				if (!strcmp(args[0],"help")) {
					args[0] = "more";
					args[1] = readmepath;
					args[2] = NULL;
				} else
				// "quit" command
				if (!strcmp(args[0],"quit")) {
					break;
				}
				//else pass command on to OS shell

				if (execargs)
					execute(args, status);
			}
		}
	}

	// if reading from a file, close the file
	if (readingFromFile) {
		fclose(fp);
	}

	return 0; 
}

/***********************************************************************

  void check4redirection(char ** args, shellstatus *sstatus);

  check command line args for i/o redirection symbols.
  set flags etc in *sstatus as appropriate

 ***********************************************************************/

void check4redirection(char ** args, shellstatus *sstatus)
{
	sstatus->infile = NULL;      // set defaults
	sstatus->outfile = NULL;
	sstatus->outmode = NULL;

	while (*args) {
		if (!strcmp(*args, "<")) {// input redirection
			*args = NULL;         // delimit args
			if(*(++args)) {       // file exist and permission OK?
				if(access(*args, F_OK)) {
					errmsg(*args,"does not exist for i/p redirection.");
				} else if (access(*args, R_OK)) {
					errmsg(*args,"is not readable for i/p redirection.");
				} else {
					sstatus->infile = *args;
				}
			} else {
				errmsg("no filename with i/p redirection symbol.", NULL);
				break;             // reached end of args
			}   
		} else if (!strcmp(*args, ">") || !strcmp(*args, ">>")) {
			if (!strcmp(*args, ">")) { // simple output redirection
				sstatus->outmode = "w";
			} else {		    // output redirection - append */
				sstatus->outmode = "a";
			}
			*args = NULL;           // delimit args
			if(*(++args)) {         // permission OK ?
				if(!access(*args, W_OK) || access(*args, F_OK)) {
					sstatus->outfile = *args;
				} else {
					errmsg(*args,"is not writable/creatable for o/p redirection.");
				}
			} else {
				errmsg("no filename with o/p redirection symbol.", NULL);
				break;
			}
		}
		args++;
	}
}

/***********************************************************************

  void execute(char ** args, shellstatus sstatus);

  fork and exec the program and command line arguments in args
  if foreground flag is TRUE, wait until pgm completes before
  returning

 ***********************************************************************/

void execute(char ** args, shellstatus sstatus)
{
	int status;
	pid_t child_pid;

	switch (child_pid = fork()) {
		case -1:
			syserrmsg("fork",NULL);
			break;
		case 0:

			// execution in child process

			// i/o redirection */

			if (sstatus.infile) {
				if (!freopen(sstatus.infile, "r", stdin))
					errmsg(sstatus.infile,"can't be open for i/p redirection.");
			}
			if (sstatus.outfile) {
				if (!freopen(sstatus.outfile, sstatus.outmode, stdout))
					errmsg(sstatus.outfile,"can't be open for o/p redirection.");
			}

			// final exec of program

			execvp(args[0], args);
			syserrmsg("exec failed -",args[0]);
			exit(127);
	}

	// continued execution in parent process

	if (sstatus.foreground) waitpid(child_pid, &status, WUNTRACED);
}

/***********************************************************************

  FILE * redirected_op(shellstatus ststus)

  return o/p stream (redirected if necessary) 

 ***********************************************************************/

FILE * redirected_op(shellstatus status)
{
	FILE * ostream = stdout;

	if (status.outfile) {
		if (!(ostream = fopen(status.outfile, status.outmode))) {
			syserrmsg(status.outfile,"can't be open for o/p redirection.");
			ostream = stdout;
		}
	}
	return ostream;
}

/********************************************************************

  void errmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr

  if msg2 is NULL only msg1 is printed
  if msg1 is NULL only "ERROR: " is printed
 *******************************************************************/

void errmsg(char * msg1, char * msg2)
{
	fprintf(stderr,"ERROR: ");
	if (msg1)
		fprintf(stderr,"%s; ", msg1);
	if (msg2)
		fprintf(stderr,"%s; ", msg2);
	return;
	fprintf(stderr,"\n");
}

/********************************************************************

  void syserrmsg(char * msg1, char * msg2);

  print an error message (or two) on stderr followed by system error
  message.

  if msg2 is NULL only msg1 and system message is printed
  if msg1 is NULL only the system message is printed
 *******************************************************************/

void syserrmsg(char * msg1, char * msg2)
{
	errmsg(msg1, msg2);
	perror(NULL);
	return;
}

