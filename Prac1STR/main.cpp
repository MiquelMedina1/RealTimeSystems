#include "Grove_LCD_RGB_Backlight.h"

#include "mbed.h"

#include <cstdint>
#include <cstdio>
#include <string>

#define DEADLINE 600

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
    if (counts < 0)
        counts = 0;
    
    return counts;
}

void read_and_comp_lux() {
    uint64_t start_read_and_comp_lux = get_ms_count();
    lux = calculate_lux();
    pot_mesurement = potentiometer.read();
    
    if(pot_mesurement < 0) {
        buzzer.write(0.25);
        buzzer_active = true;
        pot_mesurement = 0;
    }

    if (lux <= pot_mesurement)
        lux_compensate = pot_mesurement - lux;
    else
        lux_compensate = 0;
    
    led.write(lux_compensate-lux);


    printf("Time: %llu //// Potentiometer: %f\n", get_ms_count() - start_read_and_comp_lux, pot_mesurement);
}

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

void calculate_lux_mean() {
    lux_mean += lux;
    n_lux_means++;
    
    if((get_ms_count() - timer_lux_mean) >= 10000) {

        printf("Last 10s lux: %f:%f:%f", lux_mean, n_lux_means, lux_mean/n_lux_means*100);

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

            
        do_lux_mean = false;
        button.enable_irq();
        ThisThread::sleep_for(1s);      //Be able to see result
    }
}

void button_click() {
    button.disable_irq();   //We disable interrupts to avoid conflicts until the mean calculus is finished
    lux_mean = 0;
    n_lux_means = 0;
    do_lux_mean = true;
    timer_lux_mean = get_ms_count();
}

bool test_deadline() {
    if (DEADLINE < (get_ms_count() - main_timer)) {
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
    
    main_timer = get_ms_count();
    read_and_comp_lux();
    
    if(!test_deadline()) {
        
        print_lcd();

        if(do_lux_mean)
            calculate_lux_mean();

        if(!test_deadline()) {
            uint64_t time_left = DEADLINE - (main_timer - get_ms_count());
            ThisThread::sleep_for(time_left);
        }

    }
    if(buzzer_active) {
        buzzer.write(0.0);
        buzzer_active = false;
    }
  }
}