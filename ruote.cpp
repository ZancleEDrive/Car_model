#include "mbed.h"
#include <ctime>


const int fori = 20;
int counter = 0;
int counter1 = 0;
int counter2 = 0;
int counter3 = 0;
InterruptIn sensore(ARDUINO_UNO_D3);
InterruptIn sensore1(ARDUINO_UNO_D2);
InterruptIn sensore2(ARDUINO_UNO_D4);
InterruptIn sensore3(ARDUINO_UNO_D5);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
//DigitalOut flash();


void Rpm()
{
    led1 = !led1;
    counter++;
}

void Rpm1()
{
    led2 = !led2;
    counter1++;
}


void Rpm2()
{
    led3 = !led3;
    counter2++;
}



void Rpm3()
{
    counter3++;
}

double get_speed



int main(){
    sensore3.rise(&Rpm3);
    sensore2.rise(&Rpm2);
    sensore1.rise(&Rpm1);
    sensore.rise(&Rpm);  // attach the address of the flip function to the rising edge
    //sensore1.rise(&Rpm1);
    while (1) {          // wait around, interrupts will interrupt this!
       // led = !led
       ThisThread::sleep_for(250ms);
       counter=counter*12;
       counter1=counter1*12;
       counter2=counter2*12;
       counter3=counter3*12;
        printf("%d rpm  \t", counter);
        printf("%d rpm1 \t", counter1);
        printf("%d rpm2 \t", counter2);
        printf("%d rpm3 \n", counter3);
        counter3=0;
        counter2=0;
        counter1=0;
        counter=0;
       // ThisThread::sleep_for(250ms);
    }
}
