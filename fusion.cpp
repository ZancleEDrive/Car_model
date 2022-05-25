 
    /* Includes */
    #include "mbed.h"
    #include "XNucleoIKS01A2.h"
    #include <iostream>
    #include <string>
    #include <math.h>
    #define PI 2*acos(0.0f)
    #include <chrono>
    const int fori = 20;
    int counter = 0;
    int counter1 = 0;
    int counter2 = 0;
    int counter3 = 0;
    //accelerazione
    float acc_x=0;
    float acc_y=0;
    float acc_prec_x=0;
    float acc_prec_y=0;  
    //velocità
    float v_x=0;
    float v_y=0;
    //l'angolo di inclinzione si suppone costante quindi il
    //disturbo dovrà rimanere lo stesso
    double disturbo_x=0;
    double disturbo_y=0;

    InterruptIn sensore(ARDUINO_UNO_D3);
    InterruptIn sensore1(ARDUINO_UNO_D2);
    InterruptIn sensore2(ARDUINO_UNO_D6);
    InterruptIn sensore3(ARDUINO_UNO_D13);
    DigitalOut led1(LED1);
    DigitalOut led2(LED2);
    DigitalOut led3(LED3);
    /* Instantiate the expansion board */
    static XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(D14, D15, D4, D5);
    
    /* Retrieve the composing elements of the expansion board */
    static LSM303AGRAccSensor *accelerometer = mems_expansion_board->accelerometer;
    

    //calcola il disturbo sulla base dell'angolo d'inclinazione
    double calcola_disturbo(float acc){
        double angolo_inclinazione=(PI/2)-acos(acc);
        return (acc)*cos(angolo_inclinazione);
    }





    /* Helper function for printing floats & doubles */
    static char *print_double(char* str, double v, int decimalDigits=2){
        int i = 1;
        int intPart, fractPart;
        int len;
        char *ptr;
        
        /* prepare decimal digits multiplicator */
        for (;decimalDigits!=0; i*=10, decimalDigits--);
        
        /* calculate integer & fractinal parts */
        intPart = (int)v;
        fractPart = (int)((v-(double)(int)v)*i);
        
        /* fill in integer part */
        sprintf(str, "%i.", intPart);
        
        /* prepare fill in of fractional part */
        len = strlen(str);
        ptr = &str[len];
        
        /* fill in leading fractional zeros */
        for (i/=10;i>1; i/=10, ptr++) {
            if (fractPart >= i) {
                break;
            }
            *ptr = '0';
        }
        
        /* fill in (rest of) fractional part */
        sprintf(ptr, "%i", fractPart);
        
        return str;
    }


    //calcola la velocità integrando il valore dell'accelerazione, 
    //rispetto solamente al tempo trascorso per effettuare la misura
    float calcola_velocita(float acc,float tempo){
        float vel=(acc*9.80082)*(tempo/1000);
        return vel;
    }


    float filtro_kalman(float stima,float errore, float errore_misura,float misura){
    float KG=0;
    float errore_stima=errore;
    for (int i=0;i<1000;i++){  
        //Guadagno
        KG = errore_stima/(errore_stima+errore_misura);
        //stima     
        stima = stima+(KG*(misura-stima)); 
        //errore stima x
        errore_stima = (1-KG)* errore_stima;   
        
    }
        return stima;
    }




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

    double get_speed;


    //Restituisce il valore della velocità dagli rpm
    float converti_rpm(int counter){
        float raggio=0.0575;
        return raggio*(2*PI/60)*counter;


    }




   


    int main() {
        sensore3.rise(&Rpm3);
        sensore2.rise(&Rpm2);
        sensore1.rise(&Rpm1);
        sensore.rise(&Rpm);  
        uint8_t id;
        float value1, value2;
        char buffer1[32], buffer2[32],buffer3[32];
        int32_t axes[3];
    

        accelerometer->enable();
        accelerometer->read_id(&id);
        accelerometer->get_x_axes(axes);

 
        //calcola il disturbo all'inzio sapendo che il valore
        //al tempo 0 segnato dall'accelerometro deve essere 0.
        disturbo_x=calcola_disturbo(((float)axes[0])/1000);
        disturbo_y=calcola_disturbo(((float) axes[1])/1000);

        Timer clock;
        while(1) {

            // led = !led
            ThisThread::sleep_for(250ms);

            //velocità lineare delle singole ruote
            float vel1=converti_rpm(counter*12);
            float vel2=converti_rpm(counter1*12);
            float vel3=converti_rpm(counter2*12);
            float vel4=converti_rpm(counter3*12);

            //media delle velocità
            float vel=(vel1+vel2)/2;

                //printf("%d rpm  \t", counter*12);
            //printf("%d rpm1 \t", counter1*12);
            //printf("%d rpm2 \t", counter2*12);
            //printf("%d rpm3 \n", counter3*12);
            //stampa la velocità
            printf("v[m/s]: %s\n",print_double(buffer3, vel));
            counter3=0;
            counter2=0;
            counter1=0;
            counter=0;


            printf("\r\n");

        
            printf("---\r\n");

            //Parte il timer per vedere il tempo necessario per la misurazione
        // auto startTime_misura = chrono::steady_clock::now();

            accelerometer->get_x_axes(axes);

            //fine tempo
            //auto endTime_misura =  chrono::steady_clock::now();
            //tempo trascorso
            //auto elapsedTime_misura = chrono::duration_cast<chrono::milliseconds>(endTime_misura - startTime_misura).count();

            //Prende la misura dell'accelerometro e la porta in g
            //per la componente x
            acc_x= (((float)axes[0])/1000);
            //per la componente y
            acc_y = (((float)axes[1])/1000);

            //rimuove il disturbo
            acc_x=acc_x-disturbo_x;
            acc_y=acc_y-disturbo_y;

            //applica il filtro di Kalman all'accelerazione
            acc_x=filtro_kalman(acc_prec_x,0.02,0.03,acc_x);
            acc_y=filtro_kalman(acc_prec_y,0.01,0.01,acc_y);

            //calcola la velocità
            //v_x=calcola_velocita(acc_x,elapsedTime_misura);
            //v_y=calcola_velocita(acc_y,elapsedTime_misura);

            acc_prec_x=acc_x;
            acc_prec_y=acc_y;

            if(acc_x<0 && acc_y<0){
                printf("LSM303AGR [acc/g]:  -%7s, -%7s\n", print_double(buffer1, abs(acc_x)), print_double(buffer2, abs(acc_y)));
            }
            else if(acc_x<0){
                printf("LSM303AGR [acc/g]:  -%7s, %7s\n", print_double(buffer1, abs(acc_x)), print_double(buffer2, acc_y));
            }
            else{
                printf("LSM303AGR [acc/g]:  %7s, -%7s\n", print_double(buffer1, acc_x), print_double(buffer2, abs(acc_y)));
            }
            // printf("LSM303AGR [v(m/s)]:  %6f, %6f\r\n", v_x, v_y);
            ThisThread::sleep_for(1000);

        }
    }



