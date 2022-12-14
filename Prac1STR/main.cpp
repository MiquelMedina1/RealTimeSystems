#include "Grove_LCD_RGB_Backlight.h"

#include "mbed.h"

#include <string>


AnalogIn lightSensor(A0);
PwmOut led(D3);
Grove_LCD_RGB_Backlight rgbLCD(D14, D15);
Thread thread;
float lux;

float counts_ls = 0;
float calculate_lux(float counts) {
  counts = (((5 * 500) * counts) - 500) / 10;
  if (counts > 200){
      counts = 200;
  }

  return (counts < 0 ? 0 : counts);
}

void print_lcd () {
    while(true){
        string message = "LUX: " + std::to_string(lux/2) + "%" + "LUX: " + std::to_string(lux/2) + "%";
        char output[message.length() + 1];
        strcpy(output, message.c_str());

        rgbLCD.clear();
        rgbLCD.print(output);
        ThisThread::sleep_for(500ms);
    }
}

// main() runs in its own thread in the OS
int main() {
  thread.start(print_lcd);
  while (true) {
    counts_ls = lightSensor.read();
    for (int i = 0; i < 100; i++) {
      counts_ls = counts_ls + lightSensor.read();
    }
    counts_ls = counts_ls / 100;
    //printf("Counts: %f\n", counts_ls);
    lux = calculate_lux(counts_ls);
    printf("Lux: %f\n", lux);

    led.period(0.02f);
    led.write(1 - lux / 200); //Lux is our max. lux/200 = 0-1

    rgbLCD.setRGB(154, 255, 255);



    ThisThread::sleep_for(5ms);
  }
}