#include "mbed.h"
#define PI 2*acos(0.0f)

enum class axes {X, Y, Z}

#ifndef RUOTA_H
#define RUOTA_H

class Ruota{
    private:
        float       vel;
        InterruptIn sensore;
        DigitalOut  led;
        int         counter;
        float       raggio;
    public:
        Ruota(InterruptIn sensor, DigitalOut led, int counter = 0, int vel = 0);
        void            rpm();
        float           converti_rpm();
        static float    media_vel(Ruota[] ruote, int dim);
        float           get_vel();
        void            reset_counter();
}

#endif

#ifndef DISTURB_H
#define DISTURB_H

double calcola_disturbo(float acc);

#endif
