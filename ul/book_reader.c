#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define DEFAULT 0
#define READMODE 1
#define SCANMODE 2

void sighandler(int);
void sighandler_remove(int);
void printManPage(void);
char message[128];
char user_buffer[1024];
char timer_name[128];
int remove_flag = 0;
int mode = DEFAULT;
int fd; 
char user_input[132];


void sighandler(int signo);
void scanner(void);
void image_to_text(void);
void text_to_audio(void);
void reader();

int main() {
	
	int oflags;
	
	
	struct sigaction action;

	fd = open("/dev/manager", O_WRONLY);
	if (fd== -1) {
		perror("failed to open device");
		exit(EXIT_FAILURE);
	} 

	memset(&action, 0, sizeof(action));
	action.sa_handler = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	
	sigaction(SIGIO, &action, NULL);
	fcntl(fd, F_SETOWN, getpid());
	oflags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oflags | FASYNC);

	strcpy(user_input,"01");
	if(write(fd, user_input, strlen(user_input)) == -1) {
		perror("Failed to write message");
		close(fd);
		exit(EXIT_FAILURE);
	}
	printf("going to sleep!\n");
    while(1){
        pause();
    }
	
	printf("closing file\n");
	close(fd);
	
	return 0;
}




// SIGIO handler
void sighandler(int signo)
{
	strcpy(user_input,"0");
	printf("signal received\n");
	scanner();
}

// asks I2C driver to produce a jpeg file
void scanner(void){
	// we can later add button interrupt for page flip
	//mode= READMODE;
	strcpy(user_input,"10");
	if(write(fd, user_input, strlen(user_input)) == -1) {
		perror("Failed to write message");
		close(fd);
		exit(EXIT_FAILURE);
	}
	int retVal = system("libcamera-jpeg -o /home/mad-science-box/Book-Reader/ul/image.jpg");
	if (retVal == 0) {
        printf("Command executed successfully.");
    }
	printf("scanner function\n");
	image_to_text();
}

// converts the generated jpeg file to text file
void image_to_text(void){
	int retVal = system("tesseract /home/mad-science-box/Book-Reader/ul/image.jpg /home/mad-science-box/Book-Reader/ul/text");
	if (retVal == 0) {
        printf("Command executed successfully.");
    }
	printf("image_to_text\n");
	text_to_audio();
}

// converts genereate text file to audio signal
void text_to_audio(void){
    system("cat /home/mad-science-box/Book-Reader/ul/text.txt");
	int retVal = system("espeak -f /home/mad-science-box/Book-Reader/ul/text.txt -w /home/mad-science-box/Book-Reader/ul/read.wav");
	if (retVal == 0) {
        printf("Command executed successfully.");
    }
	printf("text_to_audio\n");
	reader();
}

// send audio signal to ALSA driver
void reader(){
	printf("reader\n");
	strcpy(user_input,"11");
	if(write(fd, user_input, strlen(user_input)) == -1) {
		perror("Failed to write message");
		close(fd);
		exit(EXIT_FAILURE);
	}
	int retVal = system("aplay /home/mad-science-box/Book-Reader/ul/read.wav");
	if (retVal == 0) {
        printf("Command executed successfully.");
    }
	strcpy(user_input,"01");
	if(write(fd, user_input, strlen(user_input)) == -1) {
		perror("Failed to write message");
		close(fd);
		exit(EXIT_FAILURE);
	}
    //pause();
}

