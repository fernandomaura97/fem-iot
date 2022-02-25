/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3)                                   *
 * Author: Olivier Staquet                                                    *
 * Last version available on https://github.com/ostaquet/Arduino-MQ131-driver *
 ******************************************************************************
 * MIT License
 *
 * Copyright (c) 2018 Olivier Staquet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-sensors
 * @{
 *
 * \defgroup zoul-mq131 MQ131 ozone sensor
 *
 * Driver for the MQ131 ozone sensor
 * @{
 *
 * \file
 * Header file for the MQ131 ozone sensor
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "lib/sensors.h"
#include "dev/zoul-sensors.h"
#include "dev/gpio.h"



#ifndef _MQ131_SENSOR_H_
#define _MQ131_SENSOR_H_


#define MQ131_ERROR 			(-1)
#define MQ131_SENSOR                  "MQ131 Sensor"
#define MQ131_SUCCESS                 0



// Default values
#define MQ131_DEFAULT_RL                            1000000           // Default load resistance of 1MOhms
#define MQ131_DEFAULT_STABLE_CYCLE                  15                // Number of cycles with low deviation to consider
                                                                      // the calibration as stable and reliable
#define MQ131_DEFAULT_TEMPERATURE_CELSIUS           20                // Default temperature to correct environmental drift
#define MQ131_DEFAULT_HUMIDITY_PERCENT              65                // Default humidity to correct environmental drift
#define MQ131_DEFAULT_LO_CONCENTRATION_R0           1917.22           // Default R0 for low concentration MQ131
#define MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for low concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_R0           235.00            // Default R0 for high concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for high concentration MQ131


#ifdef MQ131_CONF_PIN
#define MQ131_PIN      MQ131_CONF_PIN
#else 
#define MQ131_PIN	5
#endif
#ifdef MQ131_CONF_PORT
#define MQ131_PORT 	MQ131_CONF_PORT
#else 
#define MQ131_PORT      GPIO_A_NUM
#endif







typedef enum {LOW_CONCENTRATION, HIGH_CONCENTRATION,SN_O2_LOW_CONCENTRATION} MQ131Model;
typedef enum {PPM, PPB, MG_M3, UG_M3}MQ131Unit;


/*---------------------------------------------------------------*/
// Internal variables
// Model of MQ131
//enum MQ131Model model;

// Serial console for the debug

extern bool MQ_enableDebug;
// Details about the circuit: pins and load resistance value
unsigned int pinPower;
unsigned int pinSensor;
uint32_t valueRL;

// Timer to keep track of the pre-heating
uint32_t secLastStart;
uint32_t secToRead;

// Calibration of R0
float valueR0;

// Last value for sensor resistance
float lastValueRs;

// Parameters for environment
int temperatureCelsuis;
int humidityPercent;

/*-----------------------------------------------------------------*/


		// Initialize the driver
		void MQbegin(uint8_t _pinPower, uint8_t _pinSensor,uint32_t _RL);

		// Manage a full cycle with delay() without giving the hand back to
		// the main loop (delay() function included)
		void MQ_sample();								

		// Read the concentration of gas
		// The environment should be set for accurate results
		float MQ_getO3(MQ131Unit unit, float rs);

		// Define environment
		// Define the temperature (in Celsius) and humidity (in %) to adjust the
		// output values based on typical characteristics of the MQ131
		void MQ_setEnv(int8_t tempCels, uint8_t humPc);

		// Setup calibration: Time to read
		// Define the time to read after started the heater
		// Get function also available to know the value after calibrate()
		// (the time to read is calculated automatically after calibration)
		void MQ_setTimeToRead(uint32_t sec);
		long MQ_getTimeToRead();

		// Setup calibration: R0
		// Define the R0 for the calibration
		// Get function also available to know the value after calibrate()
		// (the time to read is calculated automatically after calibration)
		void MQ_setR0(float _valueR0);
		float MQ_getR0();

		// Launch full calibration cycle
		// Ideally, 20°C 65% humidity in clean fresh air (can take some minutes)
		// For further use of calibration values, please use getTimeToRead() and getR0()
		void MQ_calibrate();



// Internal helpers
		// Internal function to manage the heater
		void MQ_startHeater();
		bool MQ_isTimeToRead();
		void MQ_stopHeater();

		// Internal reading function of Rs
		float MQ_readRs();

		// Get environmental correction to apply on ration Rs/R0
		//float MQ_getEnvCorrectRatio(int16_t t, int16_t hum);
		float MQ_getEnvCorrectRatio();

    		// Convert gas unit of gas concentration
		
   		float MQ_convert(float input, MQ131Unit unitIn, MQ131Unit unitOut); ////unknown type name MQ131unit?
//MQ_debugStream = NULL;
    

    		



//#define MQ131_SENSOR "MQ131 sensor"
/* -------------------------------------------------------------------------- */
extern const struct sensors_sensor MQ131_sensor;
/* -------------------------------------------------------------------------- */
#endif // _MQ131_SENSOR_H_ 	

/**
 * @}
 * @}
 */


	
