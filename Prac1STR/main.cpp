#include "Grove_LCD_RGB_Backlight.h"

#include "mbed.h"

#include <cstdint>
#include <cstdio>
#include <string>

AnalogIn potenciometer(A1);
AnalogIn lightSensor(A0);
PwmOut buzzer(D5);
PwmOut led(D3);
InterruptIn button(D2);
Grove_LCD_RGB_Backlight rgbLCD(D14, D15);   //Connected to I2C
Thread thread;
float lux;
float lux_mean, n_lux_means;
bool do_lux_mean = false;
uint64_t timer_lux_mean;

float calculate_lux() {
    
    float counts = 0;
    for (int i = 0; i < 100; i++) {
      counts = counts + lightSensor.read();
    }
    counts = counts / 100;
    
    float vout = counts*5;  //5 Volts as voltage
    counts = (((5 * 500) * vout) - 500) / 10;
    counts = counts / 1212; //Set value beetwen 0-1
    
    if (counts > 1)
        counts = 1;
    if (counts < 0) //This should be tested by buzzer
        counts = 0;
    
    return counts;
}

void read_and_comp_lux() {
    //Kernel::get_ms_count();
    uint64_t start_read_and_comp_lux = get_ms_count();
    lux = calculate_lux();

    
    led.write(1-lux);

    printf("%llu\n", get_ms_count() - start_read_and_comp_lux);
}

void print_lcd() {
    string message = "LUX: " + std::to_string(lux*100) + "%";
    char output[message.length() + 1];
    strcpy(output, message.c_str());

    rgbLCD.clear();
    rgbLCD.print(output);

    rgbLCD.locate(0, 1);
    message = "RES: " + std::to_string((1 - lux)*100) + "%";
    char output2[message.length() + 1];
    strcpy(output2, message.c_str());
    rgbLCD.print(output2);
}

void calculate_lux_mean() {
    lux_mean += lux;
    n_lux_means++;
    
    if((get_ms_count() - timer_lux_mean) >= 10000){

        printf("Last 10s lux: %f:%f:%f", lux_mean, n_lux_means, lux_mean/n_lux_means*100);

        //Print LCD
        string message = "LAST 10s LUX:";
        char output[message.length() + 1];
        strcpy(output, message.c_str());

        rgbLCD.clear();
        rgbLCD.print(output);

        rgbLCD.locate(0, 1);
        message = std::to_string((lux_mean)*100) + "%";
        char output2[message.length() + 1];
        strcpy(output2, message.c_str());
        rgbLCD.print(output2);

            
        do_lux_mean = false;
        button.enable_irq();
    }
}

void button_click() {
    button.disable_irq();
    lux_mean = 0;
    n_lux_means = 0;
    do_lux_mean = true;
    timer_lux_mean = get_ms_count();
}

void test_deadline(){

}

int main() {
  buzzer.period(0.1);
  led.period(0.02f);
  button.rise(&button_click);
  rgbLCD.setRGB(154, 255, 255);

  while (true) {
    
    read_and_comp_lux();
    print_lcd();
    if(do_lux_mean)
        calculate_lux_mean();
    ThisThread::sleep_for(500ms);
  }
}