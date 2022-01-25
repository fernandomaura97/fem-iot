Bug fixes for different parts of the fem-iot project.

The dht22-fix fixes an added decimal on the results, so that it only gets one decimal digit of precision

Noise-fix uses non-blocking loop in the code using etimers in order to schedule samplings every fraction of a second, and then compute the average value of the sensor over 10 seconds. 

The hello-world program introduces an alternative method to print floats in the console correctly, choosing the number of decimals precision explicitly, by using the function PutFloat(), defined in the same file. 



