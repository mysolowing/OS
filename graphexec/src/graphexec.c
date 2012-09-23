/*
 ============================================================================
 Name        : graphexec.c
 Author      :
 Version     :
 Copyright   :
 Description :
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

// for user defined macros
#define DEFAULT_INPUT_FINE_NAME "some-graph-file.txt"
#define READ_BUFFER_SIZE 1024
#define MAX_NUM_NODE 50
#define MAX_CHILDREN_LIST_NUMBER 10

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
	int num_parent; // how many parents this node has
	int parentFinished; // how many parents have finished
	pid_t pid; // track it when it’s running
} node_t;

int makeargv(const char *s, const char *delimiters, char ***argvp);

int main(int argc, char *argv[]) {

	int i, j;
	node_t *nodes;
	nodes = (node_t*) calloc(sizeof(node_t), MAX_NUM_NODE);
	//int nodeNum=initilization(argc,&argv,&nodes)
	printf("\nCSSI 4061 Asign1 graphexec starts\n");
	for (i = 0; i < argc; ++i) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	char *inputfilename = DEFAULT_INPUT_FINE_NAME;
	if (argc >= 2) {
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

	// Reading the node connection info from the text file
	int token_num;
	int line_number = -1;
	char line_buffer[READ_BUFFER_SIZE];
	char** argvp;
	char** childrenlist;
	while (fgets(line_buffer, sizeof(line_buffer), finput) != NULL) {
		/* note that the newline is in the buffer */
		if (line_buffer[strlen(line_buffer) - 1] == '\n') {
			line_buffer[strlen(line_buffer) - 1] = '\0';
		}
		if (strcmp(line_buffer, "\0") == 0)
			continue; //get rid of blank lines
		printf("==========>%4d: \t%s\n", ++line_number, line_buffer);
		token_num = makeargv(line_buffer, ":\n", &argvp);
		if (token_num != 4) {
			perror("Invalid number of input items");
			exit(1);
		}
		nodes[line_number].id = line_number;
		strcpy(nodes[line_number].prog, argvp[0]);
		strcpy(nodes[line_number].input, argvp[2]);
		strcpy(nodes[line_number].output, argvp[3]);

		printf("ID: %d;\tProg:  %s ;\tInput:  %s;\tOutput:  %s;\t",
				nodes[line_number].id, nodes[line_number].prog,
				nodes[line_number].input, nodes[line_number].output);

		if (strcmp(argvp[1], "none") == 0) {
			nodes[line_number].num_children = 0;
			printf("NumChildNone: %d", nodes[line_number].num_children);
		} else {
			nodes[line_number].num_children = makeargv(argvp[1], " ",
					&childrenlist);
			printf("\t|%s|\t", argvp[1]);
			printf("NumChild: %d\t Children: ", nodes[line_number].num_children);
			for (i = 0; i < nodes[line_number].num_children; ++i) {
				nodes[line_number].children[i] = atoi(childrenlist[i]);
				printf("%d ", nodes[line_number].children[i]);
			}
		}
		printf("\n");

	}
	fclose(finput);

	// INITILIZATION: get nodes with no parent and put them in a queue (FIFO)
	for (i = 0; i < line_number; ++i) {
		for (j = 0; j < nodes[i].num_children; ++j) {
			nodes[nodes[i].children[j]].num_parent += 1;
		}
	}
	for (i = 0; i <= line_number; ++i) {
		printf("%d ", nodes[i].num_parent);
	}
	int first = 0, last = 0, processed = 0;
	int queue[MAX_NUM_NODE];
	memset(queue, 0, sizeof(int));
	for (i = 0; i <= line_number; ++i) {
		if (nodes[i].num_parent == 0) {
			queue[last] = i;
			++last;
		}
	}

	// When first <=last, the processing queue is non-empty, proceed forking
	int templast;	//temp the last element in order fire multiple processes
	while (first<=last){
		templast=last;
		for (i=first;i<=templast;++i){
			// 1 To fork new processes here and wait
			// 2 Update children nodes parentFinished count
			// 3 If parentFinished==num_parent, put the node to the queue
			++first;  // remember to remove this
		}
	}
	printf("\nEnd of graphexec.\n");
	return 0;
}


void initilization(int argc, char *argv[]){}

// parser a string into tokens according to delimiter given
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
	snew = s + strspn(s, delimiters); /* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;
	strcpy(t, snew);
	numtokens = 0;
	if (strtok(t, delimiters) != NULL) /* count the number of tokens in s */
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++)
			;

	/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1) * sizeof(char *))) == NULL) {
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
	*((*argvp) + numtokens) = NULL; /* put in final NULL pointer */
	return numtokens;
}
