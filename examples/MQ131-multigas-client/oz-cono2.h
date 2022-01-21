typedef struct multigas_values {
  float ppm;
  float CO;
  float no2;
  int16_t temperature;
  int16_t humidity; 
} multigas_values_t;
extern multigas_values_t sensor_values;
