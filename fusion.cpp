 
    /* Includes */
    #include "mbed.h"
    #include "XNucleoIKS01A2.h"
    #include <cmath>
    #include <iostream>
    #include <string>
    #include "src/headers/model.h"
    #include <chrono>
    
    const int fori = 20;

    int counters[4] {0};
    float acc[2] {0};
    float acc_prec[2]{0};
    float vel[2]{0};
    double disturbo[2]{0};
    float vel=0;
    float vel_prec=0;

    //l'angolo di inclinzione si suppone costante quindi il
    //disturbo dovrà rimanere lo stesso

    InterruptIn sensori[4] = {
        new InterruptIn (ARDUINO_UNO_D3),
        new InterruptIn (ARDUINO_UNO_D2),
        new InterruptIn (ARDUINO_UNO_D6),
        new InterruptIn (ARDUINO_UNO_D13)
    };
    DigitalOut led[3] = {
        new DigitalOut (LED1),    
        new DigitalOut (LED2),    
        new DigitalOut (LED3)
    };

   /* Instantiate the expansion board */
    static XNucleoIKS01A2 *mems_expansion_board = XNucleoIKS01A2::instance(D14, D15, D4, D5);
    
    /* Retrieve the composing elements of the expansion board */
    static LSM303AGRAccSensor *accelerometer = mems_expansion_board->accelerometer;
    static LSM303AGRMagSensor *magnetometer = mems_expansion_board->magnetometer;

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
        return (acc*9.80082)*(tempo/1000);
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

    double get_speed;

    int main() {
    
        Ruota ruote[4];
        uint8_t id;
        float value1, value2;
        char buffer1[32], buffer2[32],buffer3[32];
        int32_t axes_acc[3];
        int32_t axes_mag[3];
    
        //dati accelerometro
        accelerometer->enable();
        accelerometer->read_id(&id);
        accelerometer->get_x_axes(axes_acc);

        //dati magnetometro
        magnetometer->enable();
        magnetometer->read_id(&id);
        magnetometer->get_m_axes(axes_mag);

        //calcola il disturbo all'inzio sapendo che il valore
        //al tempo 0 segnato dall'accelerometro deve essere 0.
        disturbo_x=calcola_disturbo(((float)axes_acc[X])/1000);
        disturbo_y=calcola_disturbo(((float)axes_acc[Y])/1000);


        for (int i = 0; i < 4; ++i)
            ruote[i].sensore.rise(&(ruote[i].rpm()));

        Timer clock;
        while(1) {

            ThisThread::sleep_for(250ms);     
            printf("\r\n");
            printf("---\r\n");

            //Parte il timer per vedere il tempo necessario per la misurazione
            /**************************************************/
            auto startTime_misura = chrono::system_clock::now();
            /********************DA VEDERE*********************/

            accelerometer->get_x_axes(axes_acc);

            //fine tempo
            /**************************************************/
            auto endTime_misura = chrono::system_clock::now();
            /********************DA VEDERE*********************/

            //tempo trascorso
            /**************************************************/
            auto elapsedTime_misura = chrono::duration_cast<chrono::milliseconds>(endTime_misura - startTime_misura).count();
            /********************DA VEDERE*********************/

            //Prende la misura dell'accelerometro e la porta in g
            //per la componente x
            acc[X]= (((float)aaccelerometer->get_x_axe(sxes_acc))/1000);
            //per la componente y
            acc[Y] = (((float)accelerometer->get_x_axes(axes_acc))/1000);

            //rimuove il disturbo
            acc[X]=acc[X]-disturbo[X];
            acc[Y]=acc[Y]-disturbo[Y];

            //applica il filtro di Kalman all'accelerazione
            acc[X]=filtro_kalman(acc_prec[X],0.02,0.03,acc[X]);
            acc[Y]=filtro_kalman(acc_prec[Y],0.01,0.01,ac[Y]);

            //calcola la velocità
            //v_x=calcola_velocita(acc_x,elapsedTime_misura);
            //v_y=calcola_velocita(acc_y,elapsedTime_misura);

            acc_prec[X]=acc[X];
            acc_prec[Y]=acc[Y];

            printf("LSM303AGR [acc/g]:  %f, %f\n", acc_x, acc_y);
            vel = Ruota::media_vel(ruote, size(ruote));
            vel=filtro_kalman(vel_prec,0.02,0.03,vel);
            vel_prec = vel;

            for (Ruota tmp : ruote)
                tmp.reset_counter();

            printf("v[m/s]: %f",vel);

            ThisThread::sleep_for(1000);

    }

   

/*
    int main() {
  
        uint8_t id;
        float value1, value2;
        char buffer1[32], buffer2[32],buffer3[32];
        int32_t axes_acc[3];
        int32_t axes_mag[3];
    
        //dati accelerometro
        accelerometer->enable();
        accelerometer->read_id(&id);
        accelerometer->get_x_axes(axes_acc);

        //dati magnetometro
        magnetometer->enable();
        magnetometer->read_id(&id);
        magnetometer->get_m_axes(axes_mag);

        //calcola il disturbo all'inzio sapendo che il valore
        //al tempo 0 segnato dall'accelerometro deve essere 0.
        disturbo_x=calcola_disturbo(((float)axes_acc[X])/1000);
        disturbo_y=calcola_disturbo(((float) axes_acc[Y])/1000);

        Timer clock;
        while(1) {

            // led = !led
            ThisThread::sleep_for(250ms);

           


            printf("\r\n");

        
            printf("---\r\n");

            //Parte il timer per vedere il tempo necessario per la misurazione
        // auto startTime_misura = chrono::steady_clock::now();

            accelerometer->get_x_axes(axes_acc);

            //fine tempo
            //auto endTime_misura =  chrono::steady_clock::now();
            //tempo trascorso
            //auto elapsedTime_misura = chrono::duration_cast<chrono::milliseconds>(endTime_misura - startTime_misura).count();

            //Prende la misura dell'accelerometro e la porta in g
            //per la componente x
            acc_x= (((float)aaccelerometer->get_x_axe(sxes_acc))/1000);
            //per la componente y
            acc_y = (((float)accelerometer->get_x_axes(axes_acc))/1000);

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

            
            printf("LSM303AGR [acc/g]:  %f, %f\n", acc_x, acc_y);
            


             //velocità lineare delle singole ruote
            float vel1=converti_DD
            float vel2=converti_rpm(counter1*12);
            float vel3=converti_rpm(counter2*12);
            float vel4=converti_rpm(counter3*12);

            //media delle velocità (risultante)
            float vel=(vel1+vel2+vel3+vel4)/4;
            
            vel=filtro_kalman(vel_prec,0.02,0.03,vel);

            //la velocità attuale diventa quella precedente
            vel_prec=vel;



            //printf("%d rpm  \t", counter*12);
            //printf("%d rpm1 \t", counter1*12);
            //printf("%d rpm2 \t", counter2*12);
            //printf("%d rpm3 \n", counter3*12);
            //stampa la velocità
            printf("v[m/s]: %f",vel);
            counter3=0;
            counter2=0;
            counter1=0;
            counter=0;

            // printf("LSM303AGR [v(m/s)]:  %6f, %6f\r\n", v_x, v_y);
            ThisThread::sleep_for(1000);

        }
    }


*/
