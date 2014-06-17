//
//  statsh-tliu35.c
//  CS361 Homework 7
//
//  Name: Tongtong Liu
//  Net-ID: tliu35
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMD  512
#define MAX_PATH 512
#define MAX_COMMAND_SIZE 100

typedef struct elements{
	char *command;
    double userTime;
    double sysTime;
	struct elements* next;
	struct elements* prev;
}Elements;

typedef struct stats{
	struct elements* head;
	struct elements* tail;
}Stats;

/*Initializes the log.*/
void list_init(Stats* s) {
	s->head = NULL;
	s->tail = NULL;
}

/*Frees the memory associated with l */
void list_destroy(Stats* s) {
	free(s);
}

/* Adds an item to the end of the stats list */
void add_to_list(Stats* s, char *item, double usertime, double systime) {
	Elements* tmp = (Elements*)malloc(sizeof(Elements));
    
	if (tmp == NULL)
        exit(1);
    
	tmp->command = (char*)malloc(sizeof(char)*(strlen(item)+1));
    
	if (tmp->command == NULL)
        exit(1);
    
	strcpy(tmp->command, item);
    tmp->userTime = usertime;
    tmp->sysTime = systime;
    
	tmp->next = NULL;
	if (s->head == NULL) {
		tmp->prev = NULL;
		s->head = tmp;
		s->tail = s->head;
	}
	else {
		tmp->prev = s->tail;
		s->tail->next = tmp;
		s->tail = s->tail->next;
	}
}


/* Returns the number of stores elements in the list */
unsigned int list_size(Stats* s) {
	if (s->head == NULL) return 0;
	else {
		int size = 1;
		Elements* curr = s->head;
        
		while ((curr = curr->next) != NULL)
            size++;
        
		return size;
	}
}

/*Finds the first matching item on the list searching from the end.*/
char *list_search(Stats* s, const char *query) {
	char *ret;
	if (s->head == NULL) return NULL; /* size == 0 */
	else if (s->tail == NULL) { /* size == 1 */
		if (strncmp(s->head->command, query, strlen(query)) == 0) {
            
			ret = (char*)malloc(sizeof(char)*(strlen(s->head->command)+1));
            
			if (ret == NULL)
                exit(1);
            
			strcpy(ret, s->head->command);
			return ret;
		}
	}
	else { /* size > 1 */
		Elements* curr = s->tail;
		while (curr != NULL) {
			if (strncmp(curr->command, query, strlen(query)) == 0) {
				ret = (char*)malloc(sizeof(char)*(strlen(curr->command)+1));
				if (ret == NULL) exit(1);
				strcpy(ret, curr->command);
				return ret;
			}
			else curr = curr->prev;
		}
	}
    return ret;
}


void process_cmd(char* cmd,char* arg, Stats* s  ) {
    
    struct rusage rusage;
    double ut, st;
    
    
	if (strcmp(cmd, "cd") == 0) { /* cd - change directory */
		if (chdir(arg) != 0) {
			perror("Failed to change directory");
			return;
		}
		char item[MAX_CMD];
		strcpy(item, cmd);
		strcat(item, " ");
		if (arg != NULL)
            strcat(item, arg);
        
        getrusage(RUSAGE_SELF, &rusage);
        
        st = rusage.ru_stime.tv_sec + (rusage.ru_stime.tv_usec/1000000.0);
        ut = rusage.ru_utime.tv_sec + (rusage.ru_stime.tv_usec/1000000.0);
        
		add_to_list(s, item, ut, st);
	}
	else if (strcmp(cmd, "stats") == 0) { /* display command history */
        Elements* p;
        p = s->head;
        
		int i;
		for (i = 0; i < list_size(s); i++) {
			printf("%s: user time: %lf, system time: %lf\n", p->command, p->userTime, p->sysTime);
            p = p->next;
		}
	}
	else if (strcmp(cmd, "exit") == 0)
        return;
	else { /* non built-in commands */
		pid_t pid;
    
		if ((pid = fork()) < 0) { /* failed to create a process */
			perror("Failed to fork()");
			exit(1);
		}
		else if (pid == 0) { /* child process */
			if (execlp(cmd, cmd, arg, (char*)0) == -1) { /* execute the command  */
				printf("Not valid\n");
				exit(0);
			}
		}
		else if (pid > 0) { /* parent process */
			waitpid(pid, NULL, 0);
            
            getrusage(RUSAGE_SELF, &rusage);
            
            st = rusage.ru_stime.tv_sec + (rusage.ru_stime.tv_usec/1000000.0);
            ut = rusage.ru_utime.tv_sec + (rusage.ru_stime.tv_usec/1000000.0);
            
            printf("user time: %lf, system time: %lf\n",ut,st);
            
            add_to_list(s, cmd, ut, st);

		}
	}
}

int main(){
	char line[MAX_CMD];
	char path[MAX_PATH];
    
    //--------------------
    printf("\n Name: Tongtong Liu, Net_ID: tliu35\n\n");
    
	Stats* s = (Stats*)malloc(sizeof(s));
    
	if (s == NULL)
        exit(1);
    
	list_init(s);
    
	while (strcmp(line, "exit") != 0) {
		//printf("(pid=%d)%s$ ", getpid(), getcwd(path, MAX_PATH));
        printf("statsh >");
		fgets(line, MAX_CMD, stdin);
		*strchr(line, '\n') = '\0';
		if (line[0] == '\0') continue;
		char *cmd = strtok(line, " ");
		char *arg = strtok(NULL, " ");
		process_cmd(cmd, arg, s);
	}
    
    
    /* Report command history before exit */
    Elements* p;
    p = s->head;
    
    int i;
    for (i = 0; i < list_size(s); i++) {
        printf("%s: user time: %lf, system time: %lf\n", p->command, p->userTime, p->sysTime);
        p = p->next;
    }
    
    
	list_destroy(s);
	return 0;
}


