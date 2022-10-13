#include "mbed.h"


AnalogIn lightSensor(A0);
DigitalOut led(D2);


float counts_ls = 0;

float calculate_lux(float counts){
    counts = (((5*500)*counts)-500)/10;
    return ( counts < 0 ? 0 : counts);
}
// main() runs in its own thread in the OS
int main()
{
    while (true) {
        /*counts = lightSensor.read();
        for (int i = 0; i<99; i++) {
            counts = counts + lightSensor.read();
        }
        counts = counts / 100;
        */
        counts_ls = lightSensor.read();
        //printf("Counts: %f\n", counts_ls);
        float lux = calculate_lux(counts_ls);
        printf("Lux: %f\n", lux);

        led.write(1);

        ThisThread::sleep_for(1000ms);
    }
}

