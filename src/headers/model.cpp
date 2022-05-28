#include "model.h"

Ruota::Ruota(InterruptIn sensor_p, DigitalOut led_p, int counter_p = 0, int vel_p = 0, int raggio = 0.0575) {
    vel = vel_p;
    sensore = sensor_p;
    led = led_p;
    counter = counter_p;
    raggio = raggio_p;
}

void Ruota::rpm() {
    led = !led;
    ++counter;
}

//converte la velocità dagli rpm
float Ruota::converti_rpm() {
    return raggio*(2*PI/60)*counter;
}

float get_vel(){
    return vel;
}

static float Ruota::media_vel(Ruota[] ruote, int dim) {
    float sum = 0;
    for (Ruota tmp : ruote)
        sum += tmp.get_vel();
    return sum/dim;
}

void Ruota::reset_counter() {
    counter = 0;
}

//calcola il disturbo sulla base dell'angolo d'inclinazione
double calcola_disturbo(float acc) {
    return sqrt(1-pow(acc, 2));
}
