#include <ctype.h>
#include <math.h>
#include <mraa/aio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

/* GLOBALS */
/* Editable parameters by server */
static int 	PERIOD = 	3;
static bool 	STOP = 		false;  	// Opposite is START command
static bool 	CENTIGRADE = 	false;		// Default is Fahrenheit
static bool 	DISPLAY = 	false;
const  int 	MAX_BUFFER =	1024;

typedef struct {
	int m_sockfd;
	FILE *m_fout;
}globalStruct;

/* FUNCTIONS */
float getTemperature(mraa_aio_context temp) {
	const int B = 4275;
	
	uint16_t raw_data = mraa_aio_read(temp);
	float R = 1023.0 / ((float) raw_data) - 1.0;
	R *= 100000.0;

	float temperature = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15;

	/* temperature is already in Celsius */
	if(!CENTIGRADE)
		temperature = temperature * 9 / 5 + 32;

	return temperature;
}

int convertStringToInt(char *args) {
	int i = 0;
	for(i = 0; i < strlen(args); i++) {
		if(!isdigit(args[i]))
			return -1;
	}

	return atoi(args);
}

void *serverCommand(void *args) {
	globalStruct *threadArgs = args;
	char buf[MAX_BUFFER];
	while(1) {
		bzero(buf, MAX_BUFFER);
		bool invalid = false;
		int n = read(threadArgs->m_sockfd, buf, MAX_BUFFER);
		if(n < 0)
			printf("ERROR: Could not read.\n");

		if(strcmp(buf, "OFF") == 0) {
			/* You need to write for OFF */
			printf("%s\n", buf);
			fprintf(threadArgs->m_fout, "%s\n", buf);
			fclose(threadArgs->m_fout);
			close(threadArgs->m_sockfd);
			exit(0);
		}
		else if(strcmp(buf, "STOP") == 0) {
			if(!STOP)
				STOP = true;
		}
		else if(strcmp(buf, "START") == 0) {
			if(STOP)
				STOP = false;
		}
		else if(strstr(buf, "SCALE=") && strlen(buf) == 7) {
			if(buf[6] == 'C') {
				if(!CENTIGRADE)
					CENTIGRADE = true;
			}
			else if(buf[6] == 'F') {
				if(CENTIGRADE)
					CENTIGRADE = false;
			}
			else
				invalid = true;
		}
		else if(strstr(buf, "PERIOD=") && strlen(buf) <= 11) {
			int value = convertStringToInt(&buf[7]);
			if(value != -1 && value >= 0 && value <= 3600) {
				if(PERIOD != value)
					PERIOD = value;
			}
			else
				invalid = true;
		}
		else if(strstr(buf, "DISP ") && strlen(buf) == 6) {
			if(buf[5] == 'N') {
				if(DISPLAY)
					DISPLAY = false;
			}
			else if(buf[5] == 'Y') {
				if(!DISPLAY)
					DISPLAY = true;
			}
			else
				invalid = true;
		}
		else
			invalid = true;

		/* Check if the command was invalid and print to terminal and file*/
		if(invalid) {
			printf("%s I\n", buf);
			fprintf(threadArgs->m_fout, "%s I\n", buf);
		}
		else {
			printf("%s\n", buf);
			fprintf(threadArgs->m_fout, "%s\n", buf);
		}
	}
}

/* MAIN */
int main() {
	/* Initialize variables */	
	const int TEMP_SENSOR_PORT = 0;
	const int UID = 204401093;
	const int port = 16000;
	int sockfd; 
	int bytes_written = 0;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	const char *hostname = "r01.cs.ucla.edu";
	char *buf[MAX_BUFFER];
	char send_str[MAX_BUFFER];
	FILE *fout = fopen("part2.log", "w");

	/* Connect to eduroam server */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		printf("ERROR: Could not open socket.\n");
		exit(0);
	}

	/* Set values of struct to pass into thread */
	globalStruct threadArgs;
	threadArgs.m_sockfd = sockfd;
	threadArgs.m_fout = fout;

	/* Get server DNS entry */
	server = gethostbyname(hostname);
	if(server == NULL) {
		printf("ERROR: No such host as %s.\n", hostname);
		exit(0);
	}

	/* Build the server's internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, 
		(char *) &serveraddr.sin_addr.s_addr, 
		server->h_length);
	serveraddr.sin_port = htons(port);

	if(connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		printf("ERROR: Could not connect.\n");
		exit(0);
	}

	mraa_aio_context temp = mraa_aio_init(TEMP_SENSOR_PORT);

	/* Create a new thread to await command from server */
	pthread_t tid;
	pthread_create(&tid, NULL, serverCommand, (void *) &threadArgs);

	/* Send initial UID */
	bzero(send_str, MAX_BUFFER);
	bytes_written = sprintf(send_str, "%d", UID);
	if(write(sockfd, send_str, bytes_written) < 0) {
		printf("ERROR: Could not write to server.\n");
	}	

	while(1) {
		if(!STOP) {
			/* Get the temperature value from sensor. */
			float temp_value = getTemperature(temp);

			/* Get the time and format it. */
			time_t current_time = time(NULL);
			struct tm *formatted_time = localtime(&current_time);
			char time_string[10];
			memset(time_string, 0, 10);
			strftime(time_string, 9, "%H:%M:%S", formatted_time);
		
			/* Write to server */
			bzero(buf, MAX_BUFFER);
			bzero(send_str, MAX_BUFFER);
			bytes_written = sprintf(send_str, "%d TEMP=%.1f", UID, temp_value);

			if(write(sockfd, send_str, bytes_written) < 0) {
				printf("ERROR: Could not write to server.\n");
			}

			/* Print to the file. */
			printf("%s\n", send_str);
			fprintf(fout, "%s %.1f\n", time_string, temp_value);
			fflush(fout);
			sleep(PERIOD);
		}
	}

	pthread_join(tid, NULL);
	mraa_aio_close(temp);	
	fclose(fout);
	return 0;
}