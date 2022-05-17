
 
// Includes.
#include <LSM303AGR_ACC_Sensor.h>
#include <LSM303AGR_MAG_Sensor.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(ARDUINO_SAM_DUE)
#define DEV_I2C Wire1   //Define which I2C bus is used. Wire1 for the Arduino Due
#define SerialPort Serial
#else
#define DEV_I2C Wire    //Or Wire
#define SerialPort Serial
#endif



// Components.
LSM303AGR_ACC_Sensor Acc(&DEV_I2C);
LSM303AGR_MAG_Sensor Mag(&DEV_I2C);

//calcola il disturbo sulla base dell'angolo d'inclinazione
double calcola_disturbo(float acc){
  double angolo_inclinazione=(PI/2)-acos(acc);
  return (acc)*cos(angolo_inclinazione);

}

//l'angolo di inclinzione si suppone costante quindi il
//disturbo dovrà rimanere lo stesso
double disturbo_x=0;
double disturbo_y=0;

void setup() {
  // Led.
  //pinMode(13, OUTPUT);

  // Initialize serial for output.
  SerialPort.begin(9600);
  
  // Initialize I2C bus.
  DEV_I2C.begin();

  // Initlialize components.
  Acc.begin();
  Acc.Enable();
  Acc.EnableTemperatureSensor();
  Mag.begin();
  Mag.Enable();

  //prende i dati dall'accelerometro
  int32_t accelerometer[3];
  Acc.GetAxes(accelerometer);

  //calcola il disturbo all'inzio sapendo che il valore
  //al tempo 0 segnato dall'accelerometro deve essere 0.
  disturbo_x=calcola_disturbo(((float)accelerometer[0])/1000);
  disturbo_y=calcola_disturbo(((float)accelerometer[1])/1000);
}


//calcola la velocità integrando il valore dell'accelerazione, 
//rispetto solamente al tempo trascorso per effettuare la misura
float calcola_velocita(float acc,float tempo){
  float vel=(acc*9.80082)*(tempo/1000);
  return vel;

}

//Filtro di Kalman
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

 //accelerazione
    float acc_x=0;
    float acc_y=0;
    float acc_prec_x=0;
    float acc_prec_y=0;
    
    //velocità
    float v_x=0;
    float v_y=0;


void loop() {

    
    // Led blinking.
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);

    //Parte il timer per vedere il tempo necessario per la misurazione
    float startTime_misura = millis();

    // Read accelerometer LSM303AGR.
    int32_t accelerometer[3];
    Acc.GetAxes(accelerometer);

    //fine tempo
    float endTime_misura = millis();
    float elapsedTime_misura = endTime_misura - startTime_misura;
    
    
    
    //Prende la misura dell'accelerometro e la porta in g
    //per la componente x
    acc_x= (((float)accelerometer[0])/1000);
    //per la componente y
    acc_y = (((float)accelerometer[1])/1000);

    //rimuove il disturbo
    acc_x=acc_x-disturbo_x;
    acc_y=acc_y-disturbo_y;
    
    //applica il filtro di Kalman all'accelerazione
    acc_x=filtro_kalman(acc_prec_x,0.02,0.03,acc_x);
    acc_y=filtro_kalman(acc_prec_y,0.01,0.01,acc_y);

    
   
  
    //calcola la velocità
    v_x=calcola_velocita(acc_x,elapsedTime_misura);
    v_y=calcola_velocita(acc_y,elapsedTime_misura);

    acc_prec_x=acc_x;
    acc_prec_y=acc_y;

 
 // Output data.
  SerialPort.print("v_x[m/s]: ");
  SerialPort.print(v_x);
  SerialPort.print("   ");
  SerialPort.print("v_y[m/s]: ");
  SerialPort.print(v_y);
  SerialPort.print("   ");

  SerialPort.print("acc_x[g]: ");
  SerialPort.print(acc_x);
  SerialPort.print("   ");
  SerialPort.print("acc_y[g]: ");
  SerialPort.println(acc_y);

  


}
