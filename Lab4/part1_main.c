#include <math.h>
#include <mraa/aio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* GLOBALS */
const int TEMP_SENSOR_PORT = 0;

/* FUNCTIONS */
float getTemperature(mraa_aio_context temp) {
	const int B = 4275;
	
	uint16_t raw_data = mraa_aio_read(temp);
	float R = 1023.0 / ((float) raw_data) - 1.0;
	R *= 100000.0;

	float temperature = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15;
	temperature = temperature * 9 / 5 + 32;

	return temperature;
}

/* MAIN */
int main() {
	FILE *fout = fopen("part1_log.txt", "w");

	mraa_aio_context temp = mraa_aio_init(0);
	
	while(1) {
		/* Get the temperature value from sensor. */
		float temp_value = getTemperature(temp);

		/* Get the time and format it. */
		time_t current_time = time(NULL);
		struct tm *formatted_time = localtime(&current_time);
		char time_string[10];
		memset(time_string, 0, 10);
		strftime(time_string, 9, "%H:%M:%S", formatted_time);
		
		/* Print to both the terminal and file. */
		printf("%s %.1f\n", time_string, temp_value);
		fprintf(fout, "%s %.1f\n", time_string, temp_value);
		fflush(fout);
		sleep(1);
	}

	mraa_aio_close(temp);	
	fclose(fout);
}