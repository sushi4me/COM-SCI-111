#include <math.h>
#include <mraa/aio.h>

/* GLOBALS */
const int TEMP_SENSOR_PORT = 0;

/* FUNCTIONS */
float getTemperature(mraa_aio_context temp) {
	const int B = 4275;
	
	raw_data = mraa_aio_read(temp);
	float R = 1023.0 / ((float) raw_data) - 1.0;
	R *= 100000.0;

	float temperature = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15;
	temperature = temperature * 9 / 5 + 32;

	return temperature;
}

/* MAIN */
int main() {

}