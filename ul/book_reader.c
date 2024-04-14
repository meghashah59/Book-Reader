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

	memset(&action, 0, sizeof(action));
	action.sa_handler = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGIO, &action, NULL);
	fcntl(fd, F_SETOWN, getpid());
	oflags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oflags | FASYNC);


	scanner();
	sleep();

	close(fd);
	return 0;
}




// SIGIO handler
void sighandler(int signo)
{
	if (mode == READMODE){
		image_to_text();
	} else if (mode == SCANMODE){
		scanner();
	}
	

}

// asks I2C driver to produce a jpeg file
void scanner(void){
	// we can later add button interrupt for page flip
}

// converts the generated jpeg file to text file
void image_to_text(void){

	text_to_audio();
}

// converts genereate text file to audio signal
void text_to_audio(void){

	reader();
}

// send audio signal to ALSA driver
void reader(){

	sleep(); // sleep until reading
}

