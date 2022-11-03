#include "Grove_LCD_RGB_Backlight.h"
#include "mbed.h"


AnalogIn lightSensor(A0);
PwmOut led(D3);
Grove_LCD_RGB_Backlight rgbLCD(D14, D15);

float counts_ls = 0;
char currentLux[] = {'l',':',' ',' ',' ',' ','%','d',':',' ',' ',' ',' ','%',' '}; //l: 3 d: 3 --- 8 100% 100%
float calculate_lux(float counts){
    counts = (((5*500)*counts)-500)/10;
    return ( counts < 0 ? 0 : counts);
}

// main() runs in its own thread in the OS
int main()
{
    rgbLCD.setRGB(154,255,255);
    bool cond = true;
    while (true) {
        //LCD
        if(cond) {
           rgbLCD.writech('H');
           rgbLCD.writech('e');
           rgbLCD.writech('l');
           rgbLCD.writech('l');
           rgbLCD.writech('o');
           rgbLCD.writech(' ');
           rgbLCD.writech('W');
           rgbLCD.writech('o');
           rgbLCD.writech('r');
           rgbLCD.writech('l');
           rgbLCD.writech('d');
           rgbLCD.writech('!');
           cond = false;
        }
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

        led.period(0.02f);
        led.write(1-lux/200);   //Lux is our max. lux/200 = 0-1
        ThisThread::sleep_for(5ms);
    }
}