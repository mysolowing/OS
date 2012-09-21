/*
 ============================================================================
 Name        : testc.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

//for ’status’ variable:
#define INELIGIBLE 0
#define READY 1
#define RUNNING 2
#define FINISHED 3

typedef struct node {
	int id; // corresponds to line number in graph text file
	char prog[1024]; // prog + arguments
	char input[1024]; // filename
	char output[1024]; // filenameorward
	int children[10]; // children IDs
	int num_children; // how many children this node has
	pid_t pid; // track it when it’s running
} node_t;

#define DEFAULT_INPUT_FINE_NAME "some-graph-file.txt\0"
#define READ_BUFFER_SIZE 512

int makeargv(const char *s, const char *delimiters, char ***argvp);

int main(int argc, char *argv[]) {
	printf("\nCSSI 4061 Asign1 graphexec starts\n");
	int i;
	for (i = 0; i < argc; ++i) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	char *inputfilename = DEFAULT_INPUT_FINE_NAME;
	if (argc >= 2) {
		free((void *) inputfilename);
		inputfilename = argv[1];
		printf("Entered input file name %s\n", inputfilename);
	} else {
		printf("Use default file name %s\n", inputfilename);
	}

	FILE* finput;
	if (((finput = fopen(inputfilename, "r")) == 0)) {
		perror("Invalid master input file");
		exit(1);
	} else {
		printf("File open succeeded %s\n", inputfilename);
	}

	int line_number = 0;
	char line_buffer[READ_BUFFER_SIZE];
    while (fgets(line_buffer, sizeof(line_buffer), finput)!=NULL) {
        ++line_number;
        /* note that the newline is in the buffer */
        printf("%4d: \t%s", line_number, line_buffer);
    }
	printf("\n\nEnd of graphexec.\n");
	return 0;
}

int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);         /* snew is real start of string */
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t, snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

                             /* create argument array for ptrs to the tokens */
   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }
                        /* insert pointers to tokens into the argument array */
   if (numtokens == 0)
      free(t);
   else {
      strcpy(t, snew);
      **argvp = strtok(t, delimiters);
      for (i = 1; i < numtokens; i++)
          *((*argvp) + i) = strtok(NULL, delimiters);
    }
    *((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */
    return numtokens;
}