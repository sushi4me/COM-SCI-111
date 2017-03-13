#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>
#include <mraa/aio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <pthread.h>
#include <resolv.h>
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
	BIO *m_bio;
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
	if(strcmp(args, "0") == 0 ||
		strcmp(args, "00") == 0 ||
		strcmp(args, "000") == 0 ||
		strcmp(args, "0000") == 0)
		return 0;
	else {
		int value = atoi(args);
		if(value != 0)
			return value;
	}

	return -1; 
}

void *serverCommand(void *args) {
	globalStruct *threadArgs = args;
	char buf[MAX_BUFFER];
	while(1) {
		bzero(buf, MAX_BUFFER);
		bool invalid = false;
		int n = BIO_read(threadArgs->m_bio, buf, MAX_BUFFER);
		if(n < 0)
			fprintf(stderr, "ERROR: Could not read.\n");

		if(strcmp(buf, "OFF") == 0) {
			/* You need to write for OFF */
			printf("%s\n", buf);
			fprintf(threadArgs->m_fout, "%s\n", buf);
			fclose(threadArgs->m_fout);
			exit(0);
		}
		else if(strcmp(buf, "STOP") == 0)
			STOP = true;
		else if(strcmp(buf, "START") == 0)
			STOP = false;
		else if(strstr(buf, "SCALE=") && strlen(buf) == 7) {
			if(buf[6] == 'C')
				CENTIGRADE = true;
			else if(buf[6] == 'F')
				CENTIGRADE = false;
			else
				invalid = true;
		}
		else if(strstr(buf, "PERIOD=") && strlen(buf) <= 11) {
			int value = convertStringToInt(&buf[7]);
			if(value != -1 && value >= 0 && value <= 3600)
				PERIOD = value;
			else
				invalid = true;
		}
		else if(strstr(buf, "DISP ") && strlen(buf) == 6) {
			if(buf[5] == 'N')
				DISPLAY = false;
			else if(buf[5] == 'Y')
				DISPLAY = true;
			else
				invalid = true;
		}
		else
			invalid = true;

		/* Check if the command was invalid and print to both terminal and file */
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

void initializeSSL() {
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();
}

SSL_CTX *initCTX() {
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	method = TLSv1_client_method();
	ctx = SSL_CTX_new(method);
	if(ctx == NULL) {
		printf("ERROR: Could not make new SSL_CTX.\n");
		exit(0);
	}

	return ctx;
}

/* MAIN */
int main() {
	/* Initialize variables */	
	const int TEMP_SENSOR_PORT = 0;
	const int UID = 204401093;
	int bytes_written = 0;
	char *buf[MAX_BUFFER];
	char send_str[MAX_BUFFER];

	FILE *fout = fopen("part3_log.txt", "w");

	BIO *bio;
	SSL_CTX *ctx;
	SSL *ssl;

	/* Initialize SSL */
	initializeSSL();

	/* Setup SSL with BIO */
	ctx = SSL_CTX_new(SSLv23_client_method());
	
	bio = BIO_new_ssl_connect(ctx);
	BIO_get_ssl(bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname(bio, "r01.cs.ucla.edu");
	BIO_set_conn_port(bio, "17000");
	
	if(BIO_do_connect(bio) <= 0) {
        	fprintf(stderr, "Error attempting to connect\n");
        	ERR_print_errors_fp(stderr);
        	BIO_free_all(bio);
        	SSL_CTX_free(ctx);
        	return 0;
    	}

    	/* Set values of struct to pass into thread */
	globalStruct threadArgs;
	threadArgs.m_fout = fout;
	threadArgs.m_bio = bio;

	mraa_aio_context temp = mraa_aio_init(TEMP_SENSOR_PORT);

	/* Create a new thread to await command from server */
	pthread_t tid;
	pthread_create(&tid, NULL, serverCommand, (void *) &threadArgs);

	while(1) {
		if(!STOP) {
			/* Get the temperature value from sensor. */
			float temp_value = getTemperature(temp);
		
			/* Write to server */
			bzero(buf, MAX_BUFFER);
			bzero(send_str, MAX_BUFFER);
			bytes_written = sprintf(send_str, "%d TEMP=%.1f", UID, temp_value);

			if(BIO_write(bio, send_str, bytes_written) < 0) {
				fprintf(stderr, "ERROR: Could not write to server.\n");
			}

			/* Print to the file. */
			printf("%s\n", send_str);
			fprintf(fout, "%s\n", send_str);
			fflush(fout);
			sleep(PERIOD);
		}
	}

	pthread_join(tid, NULL);
	mraa_aio_close(temp);	
	fclose(fout);
	return 0;
}