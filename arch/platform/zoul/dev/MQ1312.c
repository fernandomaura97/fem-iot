
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
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-aac-sensor-test MQ131 sensor
 *
 * Demonstrates the operation of MQ131
 * @{
 *
 * \file
 *        Library for MQ131 ozone sensor, adapted from arduino MQ131 Library by oustaqet
 * 
 *
 * \author
 *         Fernando Maura
 */


#include "contiki.h"
#include "dev/MQ1312.h"
#include "lib/sensors.h"
#include "sys/rtimer.h"
#include "dev/gpio.h"
#include <math.h> 
#include "dev/ioc.h"
#include "dev/watchdog.h"
#include "dev/adc-sensors.h"
#include "dev/zoul-sensors.h"
#include "adc-zoul.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



/*---------------------------------------------------------------------------*/
/**
 * \addtogroup zoul-mq131
 * @{
 *
 *  *
 * \file
 *  Driver for the MQ131 sensor
 *
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/

//extern const struct sensors_sensor MQ131;


/*static rtimer_clock_t start_count, end_count, diff;
static struct rtimer rt;
rtimer_clock_t rt_now, rt_for;
static clock_time_t ct;
*/

#define MQ131_PORT_BASE          GPIO_PORT_TO_BASE(MQ131_PORT)
#define MQ131_PIN_MASK           GPIO_PIN_MASK(MQ131_PIN)



#define ADC_PIN      3

unsigned int pinPower = -1;
uint32_t valueRL = -1;
unsigned int pinsSensor= -1;

int temperatureCelsuis  = MQ131_DEFAULT_TEMPERATURE_CELSIUS;
int humidityPercent = MQ131_DEFAULT_HUMIDITY_PERCENT;

uint32_t delta = 15000;


                      
/**
 * Constructor, nothing special to do
 */

	
bool MQ_enableDebug = true; //enable debug to 1
	

/**
 * Init core variables
 */
void MQbegin(uint8_t _pinPower, uint8_t _pinSensor, uint32_t _RL) { 
  // Define if debug is requested
  
  
 	// Setup the model
 	
 	// Store the circuit info (pin and load resistance)
 	pinPower = _pinPower;
 	pinSensor = _pinSensor;
 	valueRL = _RL;
	// Setup default calibration value
 
 // LOW_CONCENTRATION :
  MQ_setR0(MQ131_DEFAULT_LO_CONCENTRATION_R0);
  MQ_setTimeToRead(MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ);
  /*case HIGH_CONCENTRATION :
  setR0(MQ131_DEFAULT_HI_CONCENTRATION_R0);
  setTimeToRead(MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ);
  break; 
  case SN_O2_LOW_CONCENTRATION:
  // Not tested by @ostaquet (I don't have this type of sensor)
  setR0(MQ131_DEFAULT_LO_CONCENTRATION_R0);
  setTimeToRead(MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ);
  break;
  */

	GPIO_SET_OUTPUT(MQ131_PORT_BASE, MQ131_PIN_MASK);
	GPIO_SET_PIN(MQ131_PORT_BASE, MQ131_PIN_MASK);

	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);; //configurar adc en pin elegido

	// Switch off the heater as default status
  GPIO_WRITE_PIN(MQ131_PORT_BASE, MQ131_PIN_MASK, 0);
	/* ---------------------------------------------------------------------------*/
  }
  /* -------------------------------------------------------------------------- */

  /* -------------------------------------------------------------------------- */

 /* Do a full cycle (heater, reading, stop heater)
 * The function gives back the hand only at the end
 * of the read cycle!
 */
void MQ_sample() {
	 	
 	MQ_startHeater();
 	
 	while(!MQ_isTimeToRead()) {
 		//clock_delay_usec(CLOCK_SECOND);
		clock_wait(0.1*CLOCK_SECOND);
 		}
 	
	lastValueRs = MQ_readRs();
 	
 	MQ_stopHeater();
 	}

 /**
 * Start the heater
 */
void MQ_startHeater() {
 	//digitalWrite(pinPower, HIGH); //DIGITAL WRITE ARDUINO, CONTIKI?
	GPIO_WRITE_PIN(MQ131_PORT_BASE, MQ131_PIN_MASK, 1);  


 	//secLastStart = millis()/1000; //MILLIS
	//secLastStart = RTIMER_NOW();
	secLastStart = clock_seconds(); //Contiki-ng timer
	
 }
	/**
 * Check if it is the right time to read the Rs value
 */
bool MQ_isTimeToRead() {
 	// Check if the heater has been started...
 	if(secLastStart < 0) {
 		return false;
 	}
 	// OK, check if it's the time to read based on calibration parameters
	uint32_t buff = MQ_getTimeToRead();
 	if(clock_seconds() >= secLastStart + buff ) {        
 		return true;
 	}
 	return false;
 } 

	/**
 * Stop the heater
 */
void MQ_stopHeater() {
 	//digitalWrite(pinPower, LOW); //DIGITALWRITE?????
	GPIO_WRITE_PIN(MQ131_PORT_BASE, MQ131_PIN_MASK, 0);  

 	secLastStart = -1;
 }

/**
* Get parameter time to read
*/
long MQ_getTimeToRead() {
	return secToRead;
	}

/**
 * Set parameter time to read (for calibration or to recall
 * calibration from previous run)
 */
void MQ_setTimeToRead(uint32_t sec) { 	secToRead = sec;  }

  /**
   * Read Rs value
   */

float MQ_readRs() {

	uint16_t valueSensor= adc_zoul.value(ZOUL_SENSORS_ADC3);
	float vRL = (float)valueSensor/ 16384.0 * 5.0; //16384 bits en adc cc2538
	if(!vRL) return 0.0f; //division by zero prevention
 	
	float rS = (5.0 / vRL - 1.0) * 1000000.0; // TO BE CHANGED IN CALIBRATION
	return rS;
  }

  /**
   * Set environmental values
   */

void MQ_setEnv(int8_t tempCels, uint8_t humPc) {
 	temperatureCelsuis = tempCels;
 	humidityPercent = humPc;
 }

  /**
   * Get correction to apply on Rs depending on environmental
   * conditions
   */

/*float MQ_getEnvCorrectRatio(int16_t t, int16_t hum) {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio

	uint8_t temperature = t/10; 
	uint8_t humidity = hum/10;

 	if(humidity == 60 && temperature == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	else if( humidity > 75) {
    // R^2 = 0.996
   	return -0.0103 * temperature + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	else if (humidity> 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperature + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	else{
		 return -0.0141 * temperatureCelsuis + 1.5623;
	 }	
 }

*/

float MQ_getEnvCorrectRatio() {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio
 	if(humidityPercent == 60 && temperatureCelsuis == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
    // R^2 = 0.996
   	return -0.0103 * temperatureCelsuis + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	return -0.0141 * temperatureCelsuis + 1.5623;
 }




 /**
 * Get gas concentration for O3 in ppm
 */

float MQ_getO3(MQ131Unit unit, float rs) { 
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

  float ratio = 0.0;
  //float buf_O3 = MQ_convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
  //ratio=lastValueRs / valueR0 * MQ_getEnvCorrectRatio();
  ratio = rs /  valueR0  * MQ_getEnvCorrectRatio();

  return MQ_convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
	}
 		/*case "LOW_CONCENTRATION" :
 			// Use the equation to compute the O3 concentration in ppm
		      // Compute the ratio Rs/R0 and apply the environmental correction
		      ratio = lastValueRs / valueR0 * MQ_getEnvCorrectRatio();
		      // R^2 = 0.9906
		      // Use this if you are monitoring low concentration of O3 (air quality project)	
		      float buf_conv = 	MQ_convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
		      return buf_conv
		      
		      // R^2 = 0.9986 but nearly impossible to have 0ppb
		      // Use this if you are constantly monitoring high concentration of O3
		      // return convert((10.66435681 * pow(ratio, 2.25889394) - 10.66435681), PPB, unit);

 		 case "HIGH_CONCENTRATION" :
 			// Use the equation to compute the O3 concentration in ppm
 			
		      // Compute the ratio Rs/R0 and apply the environmental correction
		      ratio = lastValueRs / valueR0 * MQ_getEnvCorrectRatio();
		      // R^2 = 0.9900
		      // Use this if you are monitoring low concentration of O3 (air quality project)
		      return MQ_convert(8.1399 * pow(ratio, 2.3297), PPM, unit);
		      
		      	// R^2 = 0.9985 but nearly impossible to have 0ppm
		      // Use this if you are constantly monitoring high concentration of O3
		      // return convert((8.37768358 * pow(ratio, 2.30375446) - 8.37768358), PPM, unit);

	    case "SN_O2_LOW_CONCENTRATION":
		      // NOT TESTED BY @ostaquet (I don't have this type of sensor)
		      ratio = 12.15* lastValueRs / valueR0 * MQ_getEnvCorrectRatio();
		      // r^2 = 0.9956
		      return MQ_convert(26.941 * pow(ratio,-1.16),PPB,unit);
		      break;
		      
		 		default :
		 			return 0.0;
		  } */

	 /**
	  * Convert gas unit of gas concentration
	  */

float MQ_convert(float input, MQ131Unit unitIn, MQ131Unit unitOut) {
  if(unitIn == unitOut) {
    return input;
  }

  float concentration = 0;

  switch(unitOut) {
    case PPM :
      // We assume that the unit IN is PPB as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPB to PPM
      return input / 1000.0;
    case PPB :
      // We assume that the unit IN is PPM as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPM to PPB
      return input * 1000.0;
    case MG_M3 :
      if(unitIn == PPM) {
        concentration = input;
      } else {
        concentration = input / 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    case UG_M3 :
      if(unitIn == PPB) {
        concentration = input;
      } else {
        concentration = input * 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    default :
      return input;
  }
	}


 /**
  * Calibrate the basic values (R0 and time to read)
  */

void MQ_calibrate() {
	 
  // Take care of the last Rs value read on the sensor
  // (forget the decimals)
  float lastRsValue = 0;
  float lastLastRsValue = 0;
  // Count how many time we keep the same Rs value in a row
  uint8_t countReadInRow = 0;
  // Count how long we have to wait to have consistent value
  uint8_t count = 0;
  

  // Get some info
  if(MQ_enableDebug) {
    printf("MQ131 : Starting calibration...\n");
    printf("MQ131 : Enable heater\n");
    printf("MQ131 : Stable cycles required : \n");
    printf("%u\n",MQ131_DEFAULT_STABLE_CYCLE);
    printf(" (compilation parameter MQ131_DEFAULT_STABLE_CYCLE)\n");
  	}

  // Start heater
  MQ_startHeater();

  uint8_t timeToReadConsistency = MQ131_DEFAULT_STABLE_CYCLE;

  while(countReadInRow <= timeToReadConsistency) {
		
		float value = MQ_readRs();

		if(MQ_enableDebug) {
      printf("MQ131 : Rs read = %lu ", (uint32_t) value); 
		      
     	//printf("%lu\n",(uint32_t)value);
      printf(" Ohms\n");
    }
   //if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value){ ////////////PROBAR PROBAR 
   if((abs(lastRsValue - value)> delta) && ((abs(lastLastRsValue - value))> delta)){ ////////////PROBAR PROBAR
	    lastLastRsValue = lastRsValue;
	    lastRsValue = value;
		if (countReadInRow<= 8){
			countReadInRow = 0;
		}
		else
			{countReadInRow = countReadInRow -5;}

      //printf("timmmmm: %u\n", countReadInRow);
      
			}	
			
	else { countReadInRow++;}
	count++;
	clock_wait(CLOCK_SECOND);
	}

  if(MQ_enableDebug) {
    printf("MQ131 : Stabilisation after \n"); 
    printf("%u\n", count);
    printf(" seconds\n");
    printf("MQ131 : Stop heater and store calibration parameters\n");
	 		}

	  // Stop heater
	  MQ_stopHeater();

	  // We have our R0 and our time to read
	  MQ_setR0(lastRsValue);
	  MQ_setTimeToRead(count);
		}

 /**
  * Store R0 value (come from calibration or set by user)
  */
void MQ_setR0(float _valueR0) {
  	valueR0 = _valueR0;
  }

 /**
 * Get R0 value
 */
 float MQ_getR0() {
 	return valueR0;
 }

static int configure(int type, int value){	

	printf("type: %d\n", type);
	printf("type: %d\n", value);

/*
	MQbegin(2,2,1000000);

	printf("Calibrando...\n");
	MQ_calibrate();
	printf("Calibrado!\n");
	printf("R0=  %f\n", MQ_getR0());
	printf("tiempo de calientamiento = %ld",MQ_getTimeToRead());
 	*/
  return MQ131_SUCCESS;
	}


static int value(int type) {
	
	//podria implementar value(int type) para hacer un case de los PPM, PPB, etc directamente, más eficiente
	
  
	printf("type: %d\n", type);
/*
	MQ_sample();
	printf("Concentration O3 : %f\n",MQ_getO3(PPM));
	printf(" ppm");
	printf("Concentration O3 : %f\n", MQ_getO3(PPB));
	printf(" ppb");
	printf("Concentration O3 : %f\n", MQ_getO3(MG_M3));
	printf("mg/m3");
	printf("Concentration O3 : %f\n ",MQ_getO3(UG_M3));
	printf("ug/m3");
	val = MQ_getO3(PPM);
	return val;

  */
  return type;
	}



SENSORS_SENSOR(MQ131_sensor, MQ131_SENSOR, value, configure, NULL);

/** @} */
//valueRL=MQ131_DEFAULT_RL;


