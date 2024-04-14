#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

void sighandler(int);
void sighandler_remove(int);
void printManPage(void);
char message[128];
char user_buffer[1024];
char timer_name[128];
int remove_flag = 0;

char *get_message(int line_num, int word_num);

int main(int argc, char **argv) {
	
	int c,fd,oflags;
	char user_input[132];
	
	struct sigaction action;
	struct sigaction sigterm_action;

	fd = open("/dev/mytimer", O_WRONLY);
	if (fd== -1) {
		perror("failed to open device");
		exit(EXIT_FAILURE);
	} 



	if (strcmp(get_message(3,1),"") != 0) {

		strcpy(message,get_message(5,1));
		
		
	} else {
		// printf("debug 3\n");
		memset(message, '\0', sizeof(message));
		//printf("message: %s\n",message);
		// printf("debug 4\n");
	}

	while ((c=getopt(argc,argv, "lsrm :")) != -1) {
		

		switch (c) {
			case 's':
				
				memset(&action, 0, sizeof(action));
				action.sa_handler = sighandler;
				action.sa_flags = SA_SIGINFO;
				sigemptyset(&action.sa_mask);
				
				sigaction(SIGTERM, &action, NULL);
				sigaction(SIGIO, &action, NULL);
				fcntl(fd, F_SETOWN, getpid());
				oflags = fcntl(fd, F_GETFL);
				fcntl(fd, F_SETFL, oflags | FASYNC);
				

				
				//if (strcmp(argv[3],message)!=0 && strcmp(message,"")!=0){
				strcpy(message,get_message(5,1));
				// printf("%s, %s, %d\n",argv[3],message,strcmp(argv[3],message));
				if (strcmp(message,"") != 0 && strcmp(argv[3],message) != 0) {
					printf("Cannot add another timer!\n");
					exit(0);
				} else {
					//strcpy(message, argv[3]);
					strcpy(user_input, argv[2]);
					strcat(user_input, " ");
					strcat(user_input,argv[3]);

					strcpy(message,get_message(5,1));
					if (strcmp(message,"") != 0  && strcmp(argv[3],message) == 0) {
						printf("The timer %s was updated\n",message);
						exit(0);
					}
					
					if(write(fd, user_input, strlen(user_input)) == -1) {
						perror("Failed to write message");
						close(fd);
						exit(EXIT_FAILURE);
					}
					strcpy(timer_name,argv[3]);

					
					
					pause();
				}
				
				exit(0);
				
			case 'm':
				
				printf("Error: multiple timers not supported\n");
			case 'l':
				

				if (strcmp(message,"") != 0 ) {
					printf("%s %s\n", message,get_message(6,1) );
				}

				
				exit(0);

			case 'r':
				// fcntl(fd, F_SETFL, oflags & ~FASYNC);
				memset(&action, 0, sizeof(action));
				sigaction(SIGIO, &action,NULL);
				memset(&action, 0, sizeof(action));
				action.sa_handler = sighandler_remove;
				action.sa_flags = SA_SIGINFO;
				sigemptyset(&action.sa_mask);
				sigaction(SIGTERM, &action, NULL);
				sigaction(SIGIO, &action, NULL);
				
				
				fcntl(fd, F_SETOWN, getpid());
				oflags = fcntl(fd, F_GETFL);
				fcntl(fd, F_SETFL, oflags | FASYNC);
				
				strcpy(timer_name,"");
				strcpy(user_input,"");
				if(write(fd, user_input, strlen(user_input)) == -1) {
					perror("Failed to write message");
					close(fd);
					exit(EXIT_FAILURE);
				}
				
				strcpy(timer_name,"");
				
				break;


			default:
				printManPage();
				exit(1);
		}
	}

	close(fd);
	return 0;
}

void printManPage() {
	printf("Error: invalid use.\n");
	printf(" myreader [-flag] [message]\n");
	printf(" -l: list timers from timer module\n");	
	printf(" -s [sec] [MSG]: write [expiration time] [message] to the mytimer module\n");
	//printf(" -m [COUNT]: write [count] to the mytimer module\n");
}


// SIGIO handler
void sighandler(int signo)
{
	// if(strcmp(get_message(3,1),"")!=0){
	printf("%s timer expired!\n", timer_name);
	// printf("SIGIO: %d\n",signo);
	memset(timer_name, '\0', sizeof(timer_name));
	exit(0);

}

void sighandler_remove(int signo)
{
	memset(timer_name, '\0', sizeof(timer_name));
	exit(0);
	
}

char *get_message(int line_num, int word_num)
{
	int pfile;
	pfile = open("/proc/mytimer", O_RDONLY);
    if (pfile == -1) {
        perror("Error opening file");
        return "";
    }

	memset(user_buffer, '\0', sizeof(message));

	read(pfile, user_buffer, 1024);

	char *line = strtok(user_buffer, "\n");
	for (int i = 0; i < line_num-1; i++) {
        line = strtok(NULL, "\n");
		//printf("line : %s\n",line);
        if (line == NULL) {
            //perror("empty line.\n");
            close(pfile);
            return "";
        }
    }

	// Tokenize the fourth line and extract the fifth word
    char *word = strtok(line, "-");

    close(pfile);

	if (word == NULL) {
		return "";
	}else {
    	return word;
	}
}
