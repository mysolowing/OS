/* CSci4061 F2012 Assignment 4
* section: 3
* login: schm2225
* date: 12/3/12
* names: Aaron Schmitz, Weichao Xu
* id: 3891645, 4284387
*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include "textbuff.h"

#define CTRL(c) ((c) & 037)
#define TEMPCHAR '~'

#define min(X, Y)                \
     ({ typeof (X) x_ = (X);          \
        typeof (Y) y_ = (Y);          \
        (x_ < y_) ? x_ : y_; })


// Defined constants to be used with the message data-structure
#define EDIT 0
#define SAVE 1
#define QUIT 2
#define DEL  3

// Maximum length of the message passing queue
#define QUEUEMAX 20


// Data structure for message passing queue used to communicate
// between the router and UI thread
struct message_t{
    int data;
    int row;
    int col;
    int command;
    struct message_t *next;
} typedef message;

message* first;
message* last;
int count;

// mutex to control access to the text buffer
pthread_mutex_t text_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t back = PTHREAD_MUTEX_INITIALIZER;

char* backup;

// The current position of the cursor in the screen
int row;
int col;

// Lines visible in the current view of textbuff
// visible on the screen
int view_min;
int view_max;

/**
 * Removes the first message from the message queue
 * and returns it.
 */
message* pop(){
	if (first != NULL)
	{
		pthread_mutex_lock(&queue);
		message* ret = first;
		first = first->next;
		count--;
		pthread_mutex_unlock(&queue);
		return ret;
	}
	return NULL;
}


/**
 * Inserts a message at the back of the message queue
 */
void push(message* m_){
	pthread_mutex_lock(&queue);
	while(count >= QUEUEMAX)
	{
		pthread_mutex_unlock(&queue);
		usleep(10);
		pthread_mutex_lock(&queue);
	}

	if (first == NULL)
	{
		first = last = m_;
		first->next = NULL;
	}
	else
	{
		last->next = m_;
		m_->next = NULL;
	}
	count++;
	pthread_mutex_unlock(&queue);
}


/**
 * Redraws the screen with lines min_line -> max_line visible on the screen
 * places the cursor at (r_, c_) when done. If insert_ is 1 then "INPUT MODE"
 * text is displayed otherwise if 0 it isn't.
 */
int redraw(int min_line, int max_line,int r_, int c_, int insert_){
    erase();
    if(max_line - min_line != LINES-1){
        perror("HELP");
        pthread_exit(NULL);
    }
    move(0,0);

    pthread_mutex_lock(&text_);

    for(;min_line < max_line;min_line++){
        char *line;
        if(getLine(min_line,&line) == 0)
            break;
        int j;
        for(j=0;j < strlen(line);j++){
            addch(*(line+j));
        }
        addch('\n');
    }

    pthread_mutex_unlock(&text_);

    if(insert_){
        standout();
        mvaddstr(LINES-1, COLS-20, "INPUT MODE");
        standend();
    }

    move(r_,c_);
    refresh();
    return 1;
}


/**
 * Input loop of the UI Thread;
 * Loops reading in characters using getch() and placing them into the textbuffer using message queue interface
 */
void input_mode(){
    int c;
    redraw(view_min, view_max, row, col, 1);
    refresh();
    for(;;){
        c = getch();
        if(c == CTRL('D')){
            break;
        }

        //Add code here to insert c into textbuff at (row, col) using the message queue interface.
		message msg;
		msg.data = c;
		msg.row = row;
		msg.col = col;
		msg.command = EDIT;
		push(&msg);

        // ------------------------------
        if(col<COLS-1){
            col++;
        }else{
            col = 0;
            row++;
        }
        if(row > LINES - 1){
            view_min++;
            view_max++;
        }
        redraw(view_min,view_max,row,col,1);
    }
    redraw(view_min,view_max,row,col,0);
}


/**
 * Main loop of the UI thread. It reads in commands as characters
 */
void loop(){
    int c;

    while(1){
        move(row,col);
        refresh();
        c = getch();

		message msg;
        switch(c){
            case 'h':
            case KEY_LEFT:
                if(col > 0)
                    col--;
                else
                    flash();
                break;
            case 'j':
            case KEY_DOWN:
                if(row < LINES -2)
                    row++;
                else
                    if(view_max+1<=getLineLength())
                        redraw(++view_min,++view_max,row,col,0);
                    else
                        flash();
                break;
            case 'k':
            case KEY_UP:
                if(row > 0)
                    row--;
                else
                    if(view_min-1 > -1)
                        redraw(--view_min,--view_max,row,col,0);
                    else
                        flash();
                break;
            case 'l':
            case KEY_RIGHT:
                if (col < COLS - 1)
                    col++;
                else
                    flash();
                break;
            case 'i':
            case KEY_IC:
                input_mode();
                break;
            case 'x':
                flash();

                // Add code here to delete character (row, col) from textbuf
								msg.row = row;
								msg.col = col;
								msg.command = DEL;
								push(&msg);
                // ------------------------------
                redraw(view_min,view_max,row,col,0);
                break;
            case 'w':
                flash();

                // Add code here to save the textbuf file
								msg.command = SAVE;
								push(&msg);
                // ------------------------------
                break;
            case 'q':
                endwin();

                // Add code here to quit the program
								msg.command = QUIT;
								push(&msg);
								pthread_exit(NULL);
                // ------------------------------
            default:
                flash();
                break;
        }

    }
}


/**
 * Function to be used to spawn the UI thread.
 */
void *start_UI(void *threadid){

    initscr();
    cbreak();
    nonl();
    noecho();
    idlok(stdscr, TRUE);
    keypad(stdscr,TRUE);

    view_min = 0;
    view_max = LINES-1;

    redraw(view_min,view_max,row,col,0);

    refresh();
    loop();

    return NULL;
}

/**
 * Function to be used to spawn the autosave thread.
 */
void *autosave(void *threadid){

    // This function loops until told otherwise from the router thread. Each loop:
	while (TRUE)
	{
		pthread_mutex_lock(&back);
        // Open the temporary file
		FILE* fout;
		if (((fout = fopen(backup, "w")) == 0)) {
			perror("Cannot save file!");
			pthread_mutex_unlock(&back);
			continue;
		}

        // Read lines from the text buffer and save them to the temporary file
		pthread_mutex_lock(&text_);
		int i, lines = getLineLength();
		for (i = 0; i < lines; i++)
		{
			char* line;
			if (getLine(i, &line))
			{
				fprintf(fout, "%s", line);
				if (i < lines - 1) fprintf(fout, "\n");
			}
		}
		pthread_mutex_unlock(&text_);

        // Close the temporary file and sleep for 5 sec.
		fclose(fout);
		pthread_mutex_unlock(&back);
		sleep(5);
	}

    return NULL;

}

int main(int argc, char **argv) {
	printf("CSCI 4601 Lab 4 Editor\n");
    row = 0;
    col = 0;

	pthread_t ui_thread, as_thread;

    // get text file from argv
	char* filename;
	if (argc == 2) {
		filename = argv[1];
	} else {
		printf("Incorrect number of arguments.\n");
		exit(1);
	}

	int len = strlen(filename);
	backup = malloc(len + 2);
	if (backup == NULL)
	{
		perror("Could not allocate memory!");
		return 0;
	}
	strcpy(backup, filename);
	backup[len] = TEMPCHAR;
	backup[len + 1] = '\0';

    // set up necessary data structures for message passing queue and initialize textbuff
	last = first = NULL;
	count = 0;
	init_textbuff(filename);

    // spawn UI thread
	int ui_id = pthread_create(&ui_thread, NULL, (void *) &start_UI, NULL);

    // spawn auto-save thread
	int as_id = pthread_create(&as_thread, NULL, (void *) &autosave, NULL);

    // Main loop until told otherwise
	bool quit = FALSE;
	while (!quit)
	{
        // Recieve messages from the message queue
		message* msg = pop();

		if (msg == NULL) continue;

		switch (msg->command)
		{
			// If EDIT then place the edits into the text buffer
			case EDIT:
				pthread_mutex_lock(&text_);
				insert(msg->row, msg->col, msg->data);
				pthread_mutex_unlock(&text_);
				break;

			// If SAVE then save the file additionally delete the temporary save
			case SAVE:
			{
				FILE* fout;
				if (((fout = fopen(filename, "w")) == 0)) {
					perror("Cannot save file!");
					continue;
				}

				int i, lines = getLineLength();
				for (i = 0; i < lines; i++)
				{
					char* line;
					if (getLine(i, &line))
					{
						fprintf(fout, "%s", line);
						if (i < lines - 1) fprintf(fout, "\n");
					}
				}
				fclose(fout);

				pthread_mutex_lock(&back);
				unlink(backup);
				pthread_mutex_unlock(&back);
				break;
			}

			// If QUIT then quit the program and tell the appropriate threads to stop
			case QUIT:
				quit = TRUE;
				break;

			// If DEL then delete the specified character from the text buffer
			case DEL:
				pthread_mutex_lock(&text_);
				deleteCharacter(msg->row, msg->col);
				pthread_mutex_unlock(&text_);
				break;
		}
	}

    // Clean up data structures
	printf("Canceling autosave thread.\n"); fflush(stdout);
	pthread_cancel(as_id);
	pthread_join(as_id, NULL);

	printf("Joinging UI thread.\n"); fflush(stdout);
	pthread_join(ui_id, NULL);

	printf("Freeing textbuff.\n"); fflush(stdout);
	deleteBuffer();

	printf("Freeing filename.\n"); fflush(stdout);
	free(backup);

	printf("Freeing queue.\n"); fflush(stdout);
	while(pop() != NULL);
    return 0;
}
