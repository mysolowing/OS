#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

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

#define DEFAULT_INPUT_FINE_NAME "some-graph-file.txt"

int main(int argc, char *argv[]){
  printf("\nCSSI 4061 Asign1 graphexec starts\n");
	int i;
	for (i=0; i < argc; ++i){
			printf("argv[%d]: %s\n",i,argv[i]);
	}
	char *inputfilename=DEFAULT_INPUT_FINE_NAME;
	if (argc>=2){
			free((void *)inputfilename);
		  inputfilename=argv[1];
		  printf("Enered input file name %s\n",inputfilename);
	} else {
			printf("Use default file name %s\n", inputfilename);
	}

	FILE *fdinput;
	if ((fdinput=fopen(inputfilename, "r")==NULL)){
			perror("Invalid master input file");
			exit(1);
	} else {
			printf("File open succeed %s\n", inputfilename);
  }

	while(fgets(line,1,sizeof(line),fp){

		}



	return 0;
}


