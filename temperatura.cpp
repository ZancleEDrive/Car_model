#include "mbed.h"
#include <cstdio>
#include <iostream>
#include <fstream>



AnalogIn s1(A2);
AnalogIn s2(A1);
     float val,tempC;

 float sensore_1 (){
        float media_1[50];
        for(int i=0; i<50; i++){
             val=s1.read();
             tempC=(val*2.8f*100);
             media_1[i]= tempC;
}
        int a=0;
        float p1,r1;
        while(a<50){
             p1+=media_1[a];
             a++;
        }
        r1= p1/50;
        HAL_Delay(2);
        return r1;
    
    }

float sensore_2 (){
        float media_2[50];
        for(int i=0; i<50; i++){
             val=s2.read();
             tempC=(val*2.8f*100);
             media_2[i]= tempC;
}
        int b=0;
        float p2,r2;
        while(b<50){
             p2+=media_2[b];
             b++;
        }
        r2= p2/50;
         HAL_Delay(2);
        return r2;
    }

int main() {
        float s1,s2;

            
     while(1){
                s1= sensore_1();
                s2= sensore_2();
            //salvare s1 e s2 in un array e fare la media di ognuno di essi 
                printf("Sensore 1: %f\t Sensore 2: %f\n", s1,s2);
                
    }
}

