#include "Grove_LCD_RGB_Backlight.h"

#include "mbed.h"

#include <cstdint>
#include <cstdio>
#include <string>

//Thist DEADLINE sets the main loop iteration time to 500ms
#define DEADLINE 500
#define READ_AND_COMP_LUX_DEADLINE 1
#define LCD_DEADLINE 12

AnalogIn potentiometer(A1);
AnalogIn lightSensor(A0);
PwmOut buzzer(D5);
PwmOut led(D3);
InterruptIn button(D2);
Grove_LCD_RGB_Backlight rgbLCD(D14, D15);   //Connected to I2C

uint64_t timer_lux_mean;
uint64_t main_timer;
float pot_mesurement;
float lux, lux_compensate;
float lux_mean, n_lux_means;
bool do_lux_mean = false;
bool buzzer_active = false;

//Timers for timelines
uint64_t start_read_and_comp_lux;
uint64_t start_print_lcd;

//Photoresistor
float calculate_lux() {
    
    float counts = 0;
    for (int i = 0; i < 100; i++) {
      counts = counts + lightSensor.read();
    }
    counts = counts / 100;
    
    float vout = counts*5;  //5 Volts as voltage
    counts = (((5 * 500) * vout) - 500) / 10;
    counts = counts / 1200; //Set value beetwen 0-1
    
    return counts;
}

//LED + potentiometer + photoresistor
void read_and_comp_lux() {
    
    lux = calculate_lux();
    pot_mesurement = potentiometer.read();
    
    if(pot_mesurement < 0) {
        buzzer.write(0.25);
        buzzer_active = true;
        pot_mesurement = 0;
    }
    if(lux < 0) {
        buzzer.write(0.25);
        buzzer_active = true;
        lux = 0;
    }
    if(lux > 1) {
        buzzer.write(0.25);
        buzzer_active = true;
        lux = 1;
    }
    
    lux_compensate = 1 - lux;

    // If potentiometer value is < than lux_compensate, we limit it to potentiometer max.
    if (lux_compensate > pot_mesurement)
        lux_compensate = pot_mesurement;
    
    led.write(lux_compensate);
}

//LCD main functionality
void print_lcd() {
    string message = "LUX: " + std::to_string(lux*100) + "%";
    char output[message.length() + 1];
    strcpy(output, message.c_str());

    rgbLCD.clear();
    rgbLCD.print(output);

    rgbLCD.locate(0, 1);
    message = "RES: " + std::to_string(lux_compensate*100) + "%";
    char output2[message.length() + 1];
    strcpy(output2, message.c_str());
    rgbLCD.print(output2);
}

//Button functionalities
void calculate_lux_mean() {
    
    lux_mean += lux;
    n_lux_means++;
    
    if((Kernel::get_ms_count() - timer_lux_mean) >= 10000) {

        printf("Last 10s lux: %f", lux_mean/n_lux_means*100);

        //Print LCD
        string message = "LAST 10s LUX:";
        char output[message.length() + 1];
        strcpy(output, message.c_str());

        rgbLCD.clear();
        rgbLCD.print(output);

        rgbLCD.locate(0, 1);
        message = std::to_string((lux_mean/n_lux_means)*100) + "%";
        char output2[message.length() + 1];
        strcpy(output2, message.c_str());
        rgbLCD.print(output2);

            
        button.enable_irq();
        Kernel::attach_idle_hook(NULL);
    }
}

void button_click() {
    button.disable_irq();   //We disable interrupts to avoid conflicts until the mean calculus is finished
    lux_mean = 0;
    n_lux_means = 0;
    do_lux_mean = true;
    timer_lux_mean = Kernel::get_ms_count();
}

//Deadlines
bool lux_deadline() {
    if (READ_AND_COMP_LUX_DEADLINE < (Kernel::get_ms_count() - main_timer)) {
        buzzer.write(0.25);
        buzzer_active = true;
        ThisThread::sleep_for(250ms);
        return true;
    }
    return false;
}

bool lcd_deadline() {
    if ((READ_AND_COMP_LUX_DEADLINE + LCD_DEADLINE) < (Kernel::get_ms_count() - main_timer)) {
        buzzer.write(0.25);
        buzzer_active = true;
        ThisThread::sleep_for(250ms);
        return true;
    }
    return false;
}

bool test_deadline() {
    if (DEADLINE < (Kernel::get_ms_count() - main_timer)) {
        buzzer.write(0.25);
        buzzer_active = true;
        ThisThread::sleep_for(250ms);
        return true;
    }
    return false;
}

int main() {
  buzzer.period(0.1);
  led.period(0.02f);
  button.rise(&button_click);
  rgbLCD.setRGB(154, 255, 255);

  while (true) {
    
    main_timer = Kernel::get_ms_count();
    
    //start_read_and_comp_lux = Kernel::get_ms_count();
    read_and_comp_lux();
    //printf("Time read_and_comp_lux: %llu\n", Kernel::get_ms_count() - start_read_and_comp_lux);
    
    if(!lux_deadline()) {
        
        //start_print_lcd = Kernel::get_ms_count();
        print_lcd();
        //printf("Time start_print_lcd: %llu\n", Kernel::get_ms_count() - start_print_lcd);

        if(!lcd_deadline()) {
            
            if(do_lux_mean) {
                do_lux_mean = false;
                Kernel::attach_idle_hook(calculate_lux_mean);
            }
            
            if(!test_deadline()) {
                uint64_t time_left = DEADLINE - (Kernel::get_ms_count() - main_timer);
                //printf("Time left: %llu//%llu\n", time_left, Kernel::get_ms_count() - main_timer);
                ThisThread::sleep_for(time_left);
            }
        }
    }
    if(buzzer_active) {
        buzzer.write(0.0);
        buzzer_active = false;
    }
  }
}