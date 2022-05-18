/**
 ******************************************************************************
 * @file    main.cpp
 * @author  CLab
 * @version V1.0.0
 * @date    2-December-2016
 * @brief   Simple Example application for using the X_NUCLEO_IKS01A1 
 *          MEMS Inertial & Environmental Sensor Nucleo expansion board.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
*/ 
 
/* Includes */
#include "mbed.h"
#include "XNucleoIKS01A2.h"
#define _USE_MATH_DEFINES
#include <math.h>
#define PI 2*acos(0.0f)
#include <chrono>
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
static char *print_double(char* str, double v, int decimalDigits=2)
{
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

 
/* Simple main function */
int main() {
  uint8_t id;
  float value1, value2;
  char buffer1[32], buffer2[32];
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
    printf("\r\n");

 
    printf("---\r\n");

    //Parte il timer per vedere il tempo necessario per la misurazione
    auto startTime_misura = chrono::steady_clock::now();

    accelerometer->get_x_axes(axes);

    //fine tempo
    auto endTime_misura =  chrono::steady_clock::now();
    //tempo trascorso
    auto elapsedTime_misura = chrono::duration_cast<chrono::milliseconds>(endTime_misura - startTime_misura).count();

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
    v_x=calcola_velocita(acc_x,elapsedTime_misura);
    v_y=calcola_velocita(acc_y,elapsedTime_misura);

    acc_prec_x=acc_x;
    acc_prec_y=acc_y;


    printf("LSM303AGR [acc/mg]:  %6f, %6f\r\n", acc_x, acc_y);

   ThisThread::sleep_for(1000);

  }
}
