 
    /* Includes */
    #include "mbed.h"
    #include "XNucleoIKS01A2.h"
    #include <cmath>
    #include <iostream>
    #include <string>
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
    float vel_prec=0;
    //l'angolo di inclinzione si suppone costante quindi il
    //disturbo dovrà rimanere lo stesso
    double disturbo_x=0;
    double disturbo_y=0;

    float deviazione_magnetica=0;
    float declinazione_magnetica=3.9;


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
    static LSM303AGRMagSensor *magnetometer = mems_expansion_board->magnetometer;

    //calcola il disturbo sulla base dell'angolo d'inclinazione
    double calcola_disturbo(float acc){
        return acc*sqrt(1-pow(acc, 2));
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

    //integra la velocità angolare
    float calcola_angolo(int vel_ang,float tempo){
      return ((float)vel_ang*0.008)*(tempo/1000);
    }

   


    int main() {
        sensore3.rise(&Rpm3);
        sensore2.rise(&Rpm2);
        sensore1.rise(&Rpm1);
        sensore.rise(&Rpm);  
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


        //calcola la direzione all'inizio per creare il sistema di riferimento
        float direzione_riferimento=angolo_direzione(axes_mag[0], axes_mag[1]);

        //calcola il disturbo all'inzio sapendo che il valore
        //al tempo 0 segnato dall'accelerometro deve essere 0.
        disturbo_x=calcola_disturbo(((float)axes_acc[0])/1000);
        disturbo_y=calcola_disturbo(((float) axes_acc[1])/1000);

        float angolo_sterzata;

        Timer clock;
        while(1) {

            // led = !led
            ThisThread::sleep_for(250ms);

            //prende la misura del magnetometro
            magnetometer->get_m_axes(axes_mag);
            //calcola l'angolo di sterzata come differenza tra la direzione
            //appena calcolata e quella di riferimento
            angolo_sterzata=angolo_direzione(axes_mag[0],axes_mag[1])-direzione_riferimento;
            

            //Parte il timer per vedere il tempo necessario per la misurazione
            // auto startTime_misura = chrono::steady_clock::now();

            //prende la misura dell'accelerometro
            accelerometer->get_x_axes(axes_acc);

            //fine tempo
            //auto endTime_misura =  chrono::steady_clock::now();
            //tempo trascorso
            //auto elapsedTime_misura = chrono::duration_cast<chrono::milliseconds>(endTime_misura - startTime_misura).count();

            //Prende la misura dell'accelerometro e la porta in g
            //per la componente x
            acc_x= (((float)axes_acc[0])/1000);
            //per la componente y
            acc_y = (((float)axes_acc[1])/1000);

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


            //velocità lineare delle singole ruote
            float vel1=converti_rpm(counter*12);
            float vel2=converti_rpm(counter1*12);
            float vel3=converti_rpm(counter2*12);
            float vel4=converti_rpm(counter3*12);

            //media delle velocità (risultante)
            float vel=(vel1+vel2+vel3+vel4)/4;
            //filtra la velocità
            vel=filtro_kalman(vel_prec,0.02,0.03,vel);
            //la velocità attuale diventa quella precedente
            vel_prec=vel;

            //printf("%d rpm  \t", counter*12);
            //printf("%d rpm1 \t", counter1*12);
            //printf("%d rpm2 \t", counter2*12);
            //printf("%d rpm3 \n", counter3*12);


            //printing dati
            printf("Angolo sterzata (°): %f",angolo_sterzata);
            printf("v[m/s]: %f",vel);
            printf("LSM303AGR [acc/g]:  %f, %f\n", acc_x, acc_y);
            counter3=0;
            counter2=0;
            counter1=0;
            counter=0;

            // printf("LSM303AGR [v(m/s)]:  %6f, %6f\r\n", v_x, v_y);
            ThisThread::sleep_for(1000);

        }
    }



